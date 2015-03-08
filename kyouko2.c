#include "kyouko2.h"

MODULE_AUTHOR("Vijay Megharaj");
MODULE_LICENSE("GPL");

unsigned int fill=0, drain=0, i;	//Do something about these guys!!
unsigned int qfull=0;

int kyouko2_probe(struct pci_dev *pci_dev, const struct pci_device_id *pci_id)
{
	unsigned int failure;
	kyouko2.pci_dev=pci_dev;
	kyouko2.p_control_base=pci_resource_start(kyouko2.pci_dev, 1);
	kyouko2.p_ram_base=pci_resource_start(kyouko2.pci_dev,2);
	kyouko2.ctl_pg_len=pci_resource_len(kyouko2.pci_dev,1);
	kyouko2.ram_pg_len=pci_resource_len(kyouko2.pci_dev,2);
//DMA
	
	failure=pci_enable_device(kyouko2.pci_dev);
	pci_set_master(kyouko2.pci_dev);	

return 0;
}


int kyouko2_open(struct inode *inode, struct file *fp)
{
	printk(KERN_ALERT "Opening Kyouko2...");
	kyouko2.k_control_base=ioremap(kyouko2.p_control_base,kyouko2.ctl_pg_len);
	kyouko2.k_ram_base=ioremap(kyouko2.p_ram_base,kyouko2.ram_pg_len);
	kyouko2.graphics_on=0;
	return 0;
}


int kyouko2_release(struct inode *inode, struct file *fp)
{

	printk(KERN_ALERT "...Releasing Kyouko2");
	iounmap(kyouko2.k_control_base);
	iounmap(kyouko2.k_ram_base);

return 0;
}


struct pci_device_id kyouko2_dev_ids[]={
	{PCI_DEVICE(PCI_VENDOR_ID_CCORSI,PCI_DEVICE_ID_CCORSI_KYOUKO2)},
	{0}
};

int kyouko2_mmap(struct file *fp, struct vm_area_struct *vma)
{
	int ret=0;
	unsigned long pba;		//Physical Base Address
	pba=(vma->vm_pgoff<<PAGE_SHIFT);
	//printk(KERN_ALERT "pba: %lx\n",pba);
	if(pba==CONTROL)
	{
	if(uid_eq(current_fsuid(),GLOBAL_ROOT_UID))	//Check if root?
	{
		ret=io_remap_pfn_range(vma,vma->vm_start,kyouko2.p_control_base>>PAGE_SHIFT, vma->vm_end-vma->vm_start, vma->vm_page_prot);
	}
	else
		printk(KERN_ALERT "You must be root to perform this Operation!!\n");
	}
	else if(pba==FRAME_BUFF)
	{
		//Not checking for root, worst a User can Do is Doodle
		ret=io_remap_pfn_range(vma,vma->vm_start,kyouko2.p_ram_base>>PAGE_SHIFT, vma->vm_end-vma->vm_start, vma->vm_page_prot);

	}
	else// if(pba==0x40000000)
	{
		ret=io_remap_pfn_range(vma,vma->vm_start,dmabuf[i].dma_handle>>PAGE_SHIFT, vma->vm_end-vma->vm_start, vma->vm_page_prot);
	printk(KERN_ALERT "Mmap done!!\n");
	}
	
return ret;	
}

void k_sync(void)
{
	while(K_READ_REG(FIFO_DEPTH)>0);	

}

void set_frame(void)
{
	K_WRITE_REG(F_COL,1024);
        K_WRITE_REG(F_ROW,768);
        K_WRITE_REG(F_PITCH,4096);
        k_sync();
        K_WRITE_REG(F_FORMAT,0xF888);
        K_WRITE_REG(F_ADD,CLR);
return;
}

void set_dac(void)
{
 K_WRITE_REG(D_WIDTH,1024);
 k_sync();
 K_WRITE_REG(D_HEIGHT,768);
 K_WRITE_REG(D_VIRTX,CLR);
 K_WRITE_REG(D_VIRTY,CLR);
 k_sync();
 K_WRITE_REG(D_FRAME,CLR);
 K_WRITE_REG(ACCN,0x40000000);  //K_WRITE_REG(ACCN,0x80000000)   --default value
 K_WRITE_REG(MOD_SET,CLR);

return;
}

void k_flush(void)
{
	K_WRITE_REG(R_FLUSH,SET);
}

//DECLARE_WAIT_QUEUE_HEAD(dma_snooze);


void initiate_transfer(void)
{
	unsigned long flags;
//	local_irq_save(flags);
	spin_lock_irqsave(&k2lock,flags);	
	if(dmabuf[fill].count==0)
	{
//		printk(KERN_ALERT "The count for this Buffer is 0!!\n");
		//local_irq_restore(flags);
		spin_unlock_irqrestore(&k2lock,flags);
		return;
	}
	if(fill==drain)
	{

//		printk(KERN_ALERT "In init_transfer\n");
//		local_irq_restore(flags);	//start
		fill=(fill+1)%NUM_BUFF;

			k_sync();
			K_WRITE_REG(B_ADDR,dmabuf[drain].dma_handle);
			K_WRITE_REG(B_CONF,dmabuf[drain].count);		
		spin_unlock_irqrestore(&k2lock,flags);
		return;
	}
	fill=(fill+1)%NUM_BUFF;
	
	if(fill==drain)
		qfull=1;	

	if(qfull)
	{
	
		spin_unlock_irqrestore(&k2lock,flags);
		wait_event_interruptible(dma_snooze,(qfull==0));
//		printk(KERN_ALERT "Putting process to sleep\n");	
		return;
	}
	
		spin_unlock_irqrestore(&k2lock,flags);
return;	
}

irqreturn_t dma_intr(int irq, void *dev_id, struct pt_regs *regs)
{
	unsigned long flags;
	spin_lock_irqsave(&k2lock,flags);	
//	printk(KERN_ALERT "In Handler\n");
	flags=K_READ_REG(INTR_STAT);
	K_WRITE_REG(INTR_STAT,0x0F);
	if((flags & 0x02)==0)
	{
		
		spin_unlock_irqrestore(&k2lock,flags);
		return IRQ_NONE;
	}
//	printk(KERN_ALERT "In hdlr:drain->%d fill->%d\n",drain,fill);
		drain=(drain+1)%NUM_BUFF;
	if((fill != drain) && qfull==0)
	{
		printk(KERN_ALERT "Draining...\n");
		K_WRITE_REG(B_ADDR,(unsigned int)dmabuf[drain].dma_handle);
		k_sync();
		K_WRITE_REG(B_CONF,dmabuf[drain].count);
	}
		
	if((fill==drain) && qfull==1)
	{
		qfull=0;
		wake_up_interruptible(&dma_snooze);
	}
	
	spin_unlock_irqrestore(&k2lock,flags);
	return (IRQ_HANDLED);	

}

long kyouko2_ioctl(struct file *fp, unsigned int cmd, unsigned long arg)
{
	
//	unsigned int one_as_int = *(unsigned int *)&one;
	switch(cmd)
	{
		case VMODE:
		{
			if(arg==GRAPHICS_ON)
			{
				printk(KERN_ALERT "Graphics On!!\n");
				kyouko2.graphics_on=1;

//Setting Frame 0			
				set_frame();
//Setting the DAC 0
				set_dac();
//Setting BG to white
				k_sync();	
				K_WRITE_FL(COLBLU,1.0);		
				K_WRITE_FL(COLGRN,1.0);
				K_WRITE_FL(COLRED,1.0);
				k_sync();
				k_flush();
				K_WRITE_REG(R_CLR,SET);
				fill=drain=0;

			}
		if(arg==GRAPHICS_OFF)
			{
				kyouko2.graphics_on=0;
				printk(KERN_ALERT "Graphics Off!!");
				K_WRITE_REG(REBOOT,SET);//Turn off
			}			
			break;
		}
	case SYNC:
		{
			printk(KERN_ALERT "Syncing...");
			k_sync();
		
			break;
		}	
	case FLUSH:
		{
			printk(KERN_ALERT "Flushing Reg...");	
			k_flush();
		break;
		}
	case START_DMA:
		{
			int ret;
	//		printk(KERN_ALERT "Start DMA!!\nfill value->%u",fill);
			
			ret=copy_from_user(&(dmabuf[drain].count), (unsigned int *)arg, sizeof(unsigned int));
			printk(KERN_ALERT "KCount->%d\n",dmabuf[fill].count);
			//return (long)dmabuf[fill].k_dma_base;
			initiate_transfer();
			
			ret=copy_to_user((unsigned int *)arg, &(dmabuf[fill].k_dma_base),sizeof(unsigned int));		//Return the next buffer
		
	//		printk(KERN_ALERT "Returning fill val\nfill value->%u",fill);
		break;
		}
	case BIND_DMA:
		{
		unsigned long dma_b = (unsigned long)0x4000;	
		int result;
		
		dma_b<<=PAGE_SHIFT;		//Page aligning
		printk(KERN_ALERT "In Bind!!\nfill->%u",fill);
		
		for(i=0;i<NUM_BUFF;i++)
		{
			dmabuf[i].k_dma_base=pci_alloc_consistent(kyouko2.pci_dev,dma_size,&(dmabuf[i].dma_handle));
		
			vm_mmap(fp, dmabuf[i].dma_handle, dma_size, PROT_READ|PROT_WRITE, MAP_SHARED, dma_b+i*dma_size);
		
		}
		//printk(KERN_ALERT "Returning from Bind:%x\n",dmabuf[0].k_dma_base);	
		
		result=copy_to_user((unsigned int *)arg, &(dmabuf[0].k_dma_base),sizeof(unsigned int));		//Instead of return just copy to user!!
	//	fill=0;
	//	return (long)dmabuf[fill].k_dma_base;


		K_WRITE_REG(INTR_STAT,0xff); 	//Check how many bits to set -- In case I screwed up and exited with bits pending
		pci_enable_msi(kyouko2.pci_dev);
		result=request_irq(kyouko2.pci_dev->irq,(irq_handler_t)dma_intr, IRQF_DISABLED|IRQF_SHARED,"dma_intr",&kyouko2);

		K_WRITE_REG(CONF_INTR,0x02);
		k_sync();
		printk(KERN_ALERT "REached here?\n");
		break;
		}
		case UNBIND_DMA:
		{
		
		while(fill != drain)
		{
			//
		}
			free_irq(kyouko2.pci_dev->irq,&kyouko2);
			pci_disable_msi(kyouko2.pci_dev);
		for(i=0;i<NUM_BUFF;i++)
		{
			vm_munmap(dmabuf[i].dma_handle, dma_size);
			pci_free_consistent(kyouko2.pci_dev,dma_size,(void *)dmabuf[i].k_dma_base,dmabuf[i].dma_handle);
		}
	

		break;
		}
		default:
		{
			printk(KERN_ALERT "Invalid IOCTL Argument\n");
		}
	}

return 0;

}


struct file_operations kyouko2_fops=
{
	.open=kyouko2_open,
	.release=kyouko2_release,
	.unlocked_ioctl=kyouko2_ioctl,
	.mmap=kyouko2_mmap,
	.owner=THIS_MODULE
};

void kyouko2_remove(struct pci_dev *pci_dev)
{
	kyouko2.pci_dev=pci_dev;
	printk(KERN_ALERT "BUUH BYE");	
		pci_disable_device(kyouko2.pci_dev);
	return;
}


struct pci_driver kyouko2_pci_drv=
{
	.name="kyouko2_PCIE",
	.id_table=kyouko2_dev_ids,
	.probe=kyouko2_probe,
	.remove=kyouko2_remove
};

struct cdev gdev;
int major=123;
int minor=246;

int my_init_function(void)
{
	int num_pci;
	num_pci=pci_register_driver(&kyouko2_pci_drv);	//Registering the PCI device
	cdev_init(&gdev,&kyouko2_fops);		//Registering the Character Driver
	cdev_add(&gdev,MKDEV(major,minor),1);
	printk(KERN_ALERT "Kyouko2 Initializing...");

return 0;
}


void my_exit_function(void)
{
	pci_unregister_driver(&kyouko2_pci_drv);
	cdev_del(&gdev);
	printk(KERN_ALERT "...Shutting Kyouko2");

}

module_init(my_init_function);
module_exit(my_exit_function);
