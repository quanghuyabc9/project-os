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

#include <linux/kallsyms.h>
#include <linux/string.h>
#include <linux/gfp.h>
#include <linux/fdtable.h>
#include <linux/path.h>
#include <linux/slab.h>

MODULE_LICENSE("GPL");

void** system_call_table_addr;
/*my custom syscall that takes process name*/
asmlinkage int (*hook_function_open)(const char* pathname, int flags);
asmlinkage int (*hook_function_write)(int fd, const void* buf, size_t count);


void fd_to_pathname(unsigned int fd, char* pathname1) {
	char* tmp;

	struct file* file;
	struct path path;
	struct files_struct* files = current->files;

	spin_lock(&files->file_lock);
	file = fcheck_files(files, fd);
	if (!file) {
		spin_unlock(&files->file_lock);
		return;
	}

	path = file->f_path;
	path_get(&file->f_path);
	spin_unlock(&files->file_lock);

	tmp = (char*)kmalloc(GFP_KERNEL, 100 * sizeof(char));
	if (!tmp) {
		path_put(&path);
		return;
	}

	char* pathname = d_path(&path, tmp, PAGE_SIZE);
	strcpy(pathname1, pathname);
	path_put(&path);

	if (IS_ERR(pathname)) {
		free_page((unsigned long)tmp);
		return;
	}

	free_page((unsigned long)tmp);
	return;
}


// Hook function, replace the system call -------------------------OPEN
// Take the same parameter as the system call OPEN
asmlinkage int function_open(const char* pathname, int flags) {
	// Print calling process name
	printk(KERN_INFO "Calling process:%s\n", current->comm);
	// Print openning file
	printk(KERN_INFO "Openning file:%s\n", pathname);
	return hook_function_open(pathname, flags);
}

// Hook function, replace the system call -------------------------WRITE
// Take the same parameter as the system call WRITE
asmlinkage int function_write(int fd, const char* buf, size_t count) {
	// Print calling process name
	printk(KERN_INFO "Calling process:%s\n", current->comm);
	char pathname[255];
	fd_to_pathname(fd, pathname);
	printk(KERN_INFO "Written file: %s\n", pathname);
	int written_bytes = hook_function_write(fd, buf, count);
	printk(KERN_INFO "Number of written bytes:%d\n", written_bytes);
	return written_bytes;
}

/*Make page writeable*/
int make_rw(unsigned long address) {

	unsigned int level;
	pte_t* pte = lookup_address(address, &level);
	if (pte->pte & ~_PAGE_RW) {
		pte->pte |= _PAGE_RW;
	}
	return 0;
}
/* Make the page write protected */
int make_ro(unsigned long address) {

	unsigned int level;
	pte_t* pte = lookup_address(address, &level);
	pte->pte = pte->pte & ~_PAGE_RW;
	return 0;
}

static int __init entry_point(void) {
	printk(KERN_INFO "Captain Hook loaded successfully..\n");
	/*MY sys_call_table address*/
	system_call_table_addr = (void*)kallsyms_lookup_name("sys_call_table");
	/* Replace custom syscall with the correct system call name (write,open,etc) to hook*/
	hook_function_open = system_call_table_addr[__NR_open];
	hook_function_write = system_call_table_addr[__NR_write];
	/*Disable page protection*/
	make_rw((unsigned long)system_call_table_addr);
	// Replace system call OPEN by our Hook function
	system_call_table_addr[__NR_open] = function_open;
	system_call_table_addr[__NR_write] = function_write;
	printk("Module had been inserted");
	return 0;
}
static void __exit exit_point(void) {
	printk(KERN_INFO "Unloaded Captain Hook successfully\n");
	/*Restore original system call */
	system_call_table_addr[__NR_open] = hook_function_open;
	system_call_table_addr[__NR_write] = hook_function_write;
	/*Renable page protection*/
	make_ro((unsigned long)system_call_table_addr);
	printk("Module had been removed");
}
module_init(entry_point);
module_exit(exit_point);
