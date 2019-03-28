
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/kernel.h>   
#include <linux/sched.h>
 
struct task_struct *task;



MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Emre Uludağ");
 
static int observerProcesses(void)
{

	for_each_process(task){
	//printk(KERN_ALERT "This is process pid \n");
}

}

static void simple_cleanup(void)
{
	printk(KERN_WARNING "bye ...\n");
}
 
module_init(observerProcesses);
module_exit(simple_cleanup);	

