#include<linux/linkage.h>
#include<linux/kernel.h>
#include<linux/syscalls.h>
#include<linux/sched.h>
#include<linux/pid.h>

SYSCALL_DEFINE1(init_sigcount, int, pid){
    struct task_struct *p;
    struct sighand_struct *sighand;
	int i=0;

        unsigned long flags;
	printk(KERN_ALERT "initialized %d\n" , pid);
	p = pid_task(find_vpid(pid),PIDTYPE_PID);
        lock_task_sighand(p,&flags);
        sighand = p->sighand; 
        for(i=0;i<64;i++){
		sighand->signalcounter[i] = 0;
	}
        unlock_task_sighand(p,&flags);
	return (1);
}




