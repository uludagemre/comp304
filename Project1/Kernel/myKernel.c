#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/pid.h>

MODULE_AUTHOR("Emre Uludag - Arda Arslan");
MODULE_LICENSE("GPL");

int processID;

module_param(processID, int, 0);

static void execute_module(struct task_struct *base_task){
	struct list_head *head;
	struct task_struct *child_task;
	int pid_of_oldest_child = 10000000;
	char name_of_oldest_child[100] = "no_child";

	list_for_each(head, &base_task->children) {
		child_task = list_entry(head, struct task_struct, sibling);
		execute_module(child_task);
		if (pid_of_oldest_child > child_task->pid) {
			pid_of_oldest_child = child_task->pid;
			strcpy(name_of_oldest_child, child_task->comm);
		}
	}
	if (pid_of_oldest_child < 10000000) {
		printk("PID of process: %d, PID of oldest child of this process: %d, Name of oldest child of this process: %s\n", base_task->pid, pid_of_oldest_child, name_of_oldest_child);
	}
}

static int load_module(void)
{
	printk("Oldestchild module is now being loaded.\n");
	if (processID == 0) {
		printk("Oldestchild module was called without providing a processID.\n");
		return 0;
	}
	else if ((find_vpid(processID) == NULL) || (pid_task(find_vpid(processID), PIDTYPE_PID) == NULL)) { 
		printk("Provided processID is not valid.\n");
		return 0;
	}
	else {
		execute_module(pid_task(find_vpid(processID), PIDTYPE_PID));
		return 0;
	}
}

static void remove_module(void)
{
	printk("Oldestchild module is now being removed.\n");
}

module_init(load_module);
module_exit(remove_module);
