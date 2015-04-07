#include<linux/linkage.h>
#include<linux/kernel.h>
#include <linux/sched.h>
#include <linux/syscalls.h>
#include<linux/pid.h>
#include<linux/types.h>
#include<linux/wait.h>

SYSCALL_DEFINE2( smunch, int, pid, unsigned long, bit_pattern) {
	struct task_struct *target;
	unsigned long flags;
	rcu_read_lock();
	target=find_task_by_vpid(pid);
	rcu_read_unlock();
	if(!target) {
	printk(KERN_ALERT "TARGET DOESNT EXIST\n"); 
	return -1;}
	
	if(!lock_task_sighand(target, &flags)) {
        printk(KERN_ALERT "Could not acquire sighand lock\n");
        return -1;}
	if(!thread_group_empty(target)) {
                printk(KERN_ALERT "Multi threaded process\n");
		unlock_task_sighand(target , &flags);
		return -1;
	}
	if((target->exit_state & (EXIT_ZOMBIE | EXIT_DEAD)) && (bit_pattern & (1<<(SIGKILL-1)))) {
		printk(KERN_ALERT " killing process with PID %d\n",pid);
		unlock_task_sighand(target, &flags);
		release_task(target);
	}
	else {
                sigaddsetmask(&(target->signal->shared_pending.signal),bit_pattern);
			wake_up_process(target);
		unlock_task_sighand(target, &flags);
	}
	return 0;
}
