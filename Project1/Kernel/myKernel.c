#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/kernel.h>   
#include <linux/sched.h>   
#include <linux/pid.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Arda Oztaskin & Melike Kavcioglu");
MODULE_DESCRIPTION("Oldestchild module for Comp304 Project1");

int processID = 0;

module_param(processID,int,0);

static void print_oldest(struct task_struct *base_task);

static int simple_init(void)
{
	// Errors should return 0/-E but it is not required by the assignment
	printk(KERN_INFO "Loading Module...\n");
	if(processID == 0){
		printk(KERN_CRIT "No processID was given to the module oldestchild\n");
		return 0;
	}
	printk(KERN_INFO "ProcessId: %d\n",processID);

	struct pid *given_pid;
	struct task_struct *base;
	given_pid = find_vpid(processID);
	if(given_pid == NULL){
		printk(KERN_CRIT "Invalid processID!\n");
		return 0;
	}
	base = pid_task(given_pid, PIDTYPE_PID);
	if(base == NULL){
		printk(KERN_CRIT "Invalid processID!\n");
		return 0;
	}
	print_oldest(base);
	return 0;
}

static void simple_cleanup(void)
{
	printk(KERN_INFO "Removing Module...\n");
}
static void print_oldest(struct task_struct *base_task){
	struct task_struct *task;
	struct list_head *list;
	int oldest_pid= 99999; // supposed to be random big number. Do not forget to change
	// in the if clause below
	
	char *oldest_name = "test";

	list_for_each(list, &base_task->children){
		task= list_entry(list, struct task_struct, sibling);
		// Task now points to one of the current's children
		//printk(KERN_INFO "PID: %d, Executable name: %s\n",task->pid,task->comm);
		print_oldest(task);
		if(task->pid < oldest_pid){
			oldest_pid = task->pid;
			oldest_name = task->comm;
		}

	}
	// If it has a child
	if(oldest_pid != 99999)
	printk(KERN_INFO "Oldest child of parent with PID %d: PID: %d,Executable name: %s\n",base_task->pid,oldest_pid,oldest_name);
	// Currently we are using the lowest PID to determine oldest child
	// Using time arithmetic would be a better approach due to PID recycling. Try it later
	
}


module_init(simple_init);
module_exit(simple_cleanup);

