// Taeeun Kim, 2017313008
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched/signal.h>
#include <linux/mm.h>
#include <linux/slab.h>

#define PSS_SHIFT 12

typedef struct node{
	struct task_struct *p;
	unsigned long vss, rss, uss;
	unsigned long long pss;
	struct node *next;
}NODE;

NODE *start;

void insert_node(struct task_struct *p, unsigned long vss, unsigned long rss, unsigned long uss, unsigned long long pss){

	NODE *cur=start;
	NODE *newNode=(NODE *)kmalloc(sizeof(NODE), GFP_KERNEL);
	newNode->p=p;
	newNode->vss=vss;
	newNode->rss=rss;
	newNode->uss=uss;
	newNode->pss=pss;

	newNode->next=NULL;

	if(start==NULL){
		start=newNode;
		newNode->next=NULL;
		return;
	}
	if(newNode->pss > start->pss){
		newNode->next=start;
		start=newNode;
		return;
	}

	while(1){
		if(cur->next==NULL){
			cur->next=newNode;
			newNode->next=NULL;
			return;
		}
		else if(cur->next->pss < newNode->pss){
			newNode->next=cur->next;
			cur->next=newNode;
			return;
		}
		else cur=cur->next;
	}
}




static int procinfo_proc_show(struct seq_file *m, void *v)
{
	struct task_struct *p;
	struct mem_size_stats mss;
	struct vm_area_struct *vma;
	
	unsigned long sharedMemory=0, vss=0, rss=0, uss=0;
	unsigned long long pss=0;

	NODE *cur;
	start=NULL;
	

	seq_printf(m, "%5s %17s %10s %10s %10s %10s\n", "PID", "Process_NAME", "VSS", "RSS", "USS", "PSS");
	seq_printf(m, "-------------------------------------------------------------------\n");

	
	for_each_process(p) // Traverse all processes
	{	
		
		memset(&mss, 0, sizeof(mss));
		vss=0, rss=0, uss=0, pss=0;
		
		if(p->mm!=NULL){
			for (vma=p->mm->mmap; vma; vma= vma->vm_next) {
				smap_gather_stats(vma, &mss);
				vss+=(vma->vm_end-vma->vm_start);
			}
			sharedMemory=mss.shared_clean+mss.shared_dirty;
			rss=mss.resident;
			uss=rss-sharedMemory;
			pss=mss.pss;
			
			
		}
		insert_node(p, vss, rss, uss, pss);
	}

	
	cur=start;

	while(cur!=NULL){
		p=cur->p;

		seq_printf(m, "%5u ", p->pid);	// print PID
		seq_printf(m, "%17s ", p->comm); // print Process_NAME
		seq_printf(m, "%9luK ", cur->vss>>10); // print VSS
		seq_printf(m, "%9luK ", cur->rss>>10); // print RSS
		seq_printf(m, "%9luK ", cur->uss>>10); // print USS
		seq_printf(m, "%9lluK\n", cur->pss>>(10+PSS_SHIFT)); // print PSS

		cur=cur->next;
	}

	return 0;
}

static int __init procrank_init(void)
{
	proc_create_single("procrank", 0, NULL, procinfo_proc_show); // create proc file
	printk("init procrank ID:2017313008\n");
	return 0;
}

static void __exit procrank_exit(void)
{
	remove_proc_entry("procrank", NULL);	// delete proc file
	printk("exit procrank ID:2017313008\n");
}

module_init(procrank_init);
module_exit(procrank_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("procrank module");
MODULE_AUTHOR("[Kim Tae Eun]");


