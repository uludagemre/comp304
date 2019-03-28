#include <linux/module.h>
#include<linux/moduleparam.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include<linux/slab.h>
#include<linux/sched.h>
#include <linux/init.h>
#include <linux/types.h>
 
MODULE_LICENSE("KOC /GPL");
MODULE_AUTHOR("Emre Uludag-Arda Arslan");
 
static int processID = -1000;
struct list_head *list;
struct task_struct *current_Task;
struct task_struct *tempProcess;
module_param(processID, int, S_IRWXU);
struct task_struct *task;


int myKernel_init(void)
{
	printk(KERN_ALERT "Trying to load the module...\n");
	int thereIsAMatchedProcess = 0;	
	if(processID ==-1000)
{
	printk(KERN_CRIT "There is no process found with given pid, operation is terminating..\n");
	return 0;
}
	for_each_process(current_Task){


		if(current_Task->pid ==processID){
			thereIsAMatchedProcess = 1;

			printk(KERN_CRIT " Process ID: %d\n", current_Task->pid);
			printk(KERN_CRIT " Parent ID: %d\n", current_Task->parent->pid);
			printk(KERN_CRIT " User ID: %d\n", current_Task ->cred->uid);
			printk(KERN_CRIT " Name: %s\n", current_Task->comm);
			printk(KERN_CRIT " Process Runtime: %s\n", CurrentTask->se.vruntime);
			printk(KERN_CRIT " Children of the process: \n")
			list_for_each(list, &current_Task->children){
				tempProcess = list_entry(list, struct task_struct, sibling);
				printk(KERN_CRIT " Child ID: %d\n", temp->pid);
				printk(KERN_CRIT " Child Name: %s\n", temp->comm);
			}
			
		}
	}
	if(thereIsAMatchedProcess == 0){
		printk(KERN_CRIT "There is no process matched with the given pid: %d",processID);
	}
	return 0;
}
 
void myKernel_exit(void)
{
	printk(KERN_WARNING "The module is being removed from the kernel\n");
}
 
module_init(myKernel_init);
module_exit(myKernel_exit);
