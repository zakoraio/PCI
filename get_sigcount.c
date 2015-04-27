#include<linux/linkage.h>
#include<linux/kernel.h>
#include<linux/syscalls.h>
#include<linux/sched.h>
#include<linux/pid.h>

SYSCALL_DEFINE1(get_sigcount, int, signumber){
	unsigned long flags;
	int count;
	struct sighand_struct *sighand;
	if(signumber < 0|| signumber > 63) {
    	return -1;
	}	
	lock_task_sighand(current , &flags);
	sighand = current->sighand;
	count = sighand->signalcounter[signumber];
	unlock_task_sighand(current, &flags);
	printk(KERN_ALERT " in get_sigcount , signum =%d , count = %d", signumber, count);
	return count;
}

