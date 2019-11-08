#include <asm/unistd.h>
#include <asm/cacheflush.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <asm/pgtable_types.h>
#include <linux/highmem.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/moduleparam.h>
#include <linux/unistd.h>
#include <asm/cacheflush.h>

void **system_call_table_addr;
asmlinkage int (*custom_syscall) (char *pathname, int flags);

/*Make page writeable*/
int make_rw(unsigned long address){
	unsigned int level;
	pte_t *pte = lookup_address(address, &level);
	if(pte->pte &~_PAGE_RW){
		pte->pte |=_PAGE_RW;
	}
	return 0;
}

/* Make the page write protected */
int make_ro(unsigned long address){
	unsigned int level;
	pte_t *pte = lookup_address(address, &level);
	pte->pte = pte->pte &~_PAGE_RW;
	return 0;
}
// Hook function, replace the system call OPEN
// Take the same parameter as the system call OPEN
asmlinkage int hook_function(const char *pathname, int flags)
{
	// Print calling process name
	printk(KERN_INFO "Calling process:%s\n",current->comm);
	// Print openning file
	printk(KERN_INFO "Openning file:%s\n",pathname);
	return custom_syscall(pathname, flags);
}

static int __init entry_point(void){
	//system call table address
	system_call_table_addr = (void*)0xffffffff81e00180;
	// Assign custom_syscall to system call OPEN
	custom_syscall = system_call_table_addr[__NR_open];
	//Disable page protection
	make_rw((unsigned long)system_call_table_addr);
	// Replace system call OPEN by our Hook function
	system_call_table_addr[__NR_open] = hook_function;
	return 0;
}

static void  __exit exit_point(void){
	// Restore system call OPEN
	system_call_table_addr[__NR_open] = custom_syscall;
	//Renable page protection
	make_ro((unsigned long)system_call_table_addr);
	return ;
}

module_init(entry_point);
module_exit(exit_point);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Quang Huy");
MODULE_DESCRIPTION("Hook-open for syscall Open");
