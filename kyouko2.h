#include<linux/init.h>
#include<linux/module.h>
#include<linux/mman.h>
#include<linux/kernel.h>
#include<linux/kernel_stat.h>
#include<linux/ioctl.h>
#include<linux/fs.h>
#include<linux/delay.h>
#include<linux/pci.h>
#include<linux/cdev.h>
#include<asm/io.h>


#define PCI_VENDOR_ID_CCORSI 0x1234
#define PCI_DEVICE_ID_CCORSI_KYOUKO2 0x1113

#define GRAPHICS_ON 1
#define GRAPHICS_OFF 0
#define VMODE _IOW(0xCC,0,unsigned long)
#define BIND_DMA _IOW(0xCC,1,unsigned long)
#define START_DMA _IOWR(0xCC,2,unsigned long)
#define SYNC _IO(0xCC,3)
#define FLUSH _IO(0xCC,4)
#define UNBIND_DMA _IO(0xCC,5)
#define FRAME_BUFF 0x80000000
#define CONTROL 0
#define ON 1
#define FIFO_DEPTH 0x4004
#define REBOOT 0x1000
#define ACCN 0x1010
#define MOD_SET 0x1008

#define B_ADDR 0x2000
#define B_CONF 0x2008

#define COLBLU 0x5100
#define COLGRN 0x5104
#define COLRED 0x5108
#define COLALP 0x510C

#define R_CLR 0x3008
#define R_FLUSH 0x3FFC

#define F_COL 0x8000
#define F_ROW 0x8004
#define F_PITCH 0x8008
#define F_FORMAT 0x800C
#define F_ADD 0x8010

#define D_WIDTH 0x9000
#define D_HEIGHT 0x9004
#define D_VIRTX 0x9008
#define D_VIRTY 0x900C
#define D_FRAME 0x9010

#define R_PRIM 0x3000
#define R_FTGT 0x3100

#define D_XCOR 0x5000
#define D_YCOR 0x5004
#define D_ZCOR 0x5008
#define D_WCOR 0x500C

#define D_BCOL 0x5010
#define D_GCOL 0x5014
#define D_RCOL 0x5018
#define D_ALPH 0x501C

#define D_XTM0 0x5080
#define D_XTM1 0x5084
#define D_XTM2 0x5088
#define D_XTM3 0x508C

#define D_YTM0 0x5090
#define D_YTM1 0x5094
#define D_YTM2 0x5098
#define D_YTM3 0x509C

#define D_ZTM0 0x50A0
#define D_ZTM1 0x50A4
#define D_ZTM2 0x50A8
#define D_ZTM3 0x50AC

#define D_WTM0 0x50B0
#define D_WTM1 0x50B4
#define D_WTM2 0x50B8
#define D_WTM3 0x50BC

#define CONF_INTR 0x100C
#define INTR_STAT 0x4008

#define R_EMIT 0x3004
#define SET 0x01
#define CLR 0x0

#define DMA_SIZE 126976
#define NUM_BUFF 8

static struct kyouko2_regs
{
        unsigned long p_control_base;
        unsigned int *k_control_base;
        unsigned long p_ram_base;
        unsigned int *k_ram_base;
        unsigned long ctl_pg_len;
        unsigned long ram_pg_len;
        struct pci_dev *pci_dev;
        int graphics_on;

}kyouko2;

struct dma_type
{
	dma_addr_t dma_handle;
	unsigned int *k_dma_base;
	unsigned int count;
}dmabuf[NUM_BUFF];	

unsigned int dma_size=DMA_SIZE;

unsigned int K_READ_REG(unsigned int reg)
{
        unsigned int value;
        udelay(100);
        rmb();
        value=*(kyouko2.k_control_base+(reg>>2));
        return (value);
}

void K_WRITE_REG(unsigned int reg, unsigned int value)
{
        *(kyouko2.k_control_base+(reg>>2))=value;
}

void K_WRITE_FL(unsigned int reg, float fl)
{
        *(kyouko2.k_control_base+(reg>>2))=*(unsigned int *)&fl;
}

DECLARE_WAIT_QUEUE_HEAD(dma_snooze);

/*
 static inline unsigned long do_mmap(struct file *file, unsigned long addr, unsigned long len, unsigned long prot, unsigned long flag, unsigned long offset) 
{
    unsigned long ret = -EINVAL;
    if ((offset + PAGE_ALIGN(len)) < offset)
        goto out;
    if (!(offset & ~PAGE_MASK))
        ret = do_mmap_pgoff(file, addr, len, prot, flag, 
                            offset >> PAGE_SHIFT);
out:
        return ret;
}*/
