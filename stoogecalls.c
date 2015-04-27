#include <linux/linkage.h>
#include <linux/kernel.h>
#include <linux/syscalls.h>
extern int my_service_sum;
extern int my_wait_sum;
extern int my_number_of_request;


SYSCALL_DEFINE1(goober, int, myarg) {
	printk(KERN_ALERT "HELLO%d\n", myarg);
	my_service_sum=0;
	my_wait_sum=0;
	my_number_of_request=0;
	return(1);
}
