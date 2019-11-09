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
#include <asm/cacheflush.h>

#include <linux/kallsyms.h>
#include <linux/string.h>
#include <linux/gfp.h>
#include <linux/fdtable.h>
#include <linux/path.h>
#include <linux/slab.h>

void **system_call_table_addr;
asmlinkage int (*custom_syscall)(int fd, const void *buf, size_t count);

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



void fd_to_pathname(unsigned int fd, char* pathname) {
	char *tmp;
        char *pathname1 = "";
 
        struct file *file;
        struct path path;
        struct files_struct *files = current->files;
 
        spin_lock(&files->file_lock);
        file = fcheck_files(files, fd);
        if (!file) {
                spin_unlock(&files->file_lock);
                return pathname;
        }
 
        path = file->f_path;
        path_get(&file->f_path);
        spin_unlock(&files->file_lock);
 
       
        tmp = (char*)kmalloc(GFP_KERNEL, 100 * sizeof(char));;
        if(!tmp){
                path_put(&path);
                return pathname;
        }
 
        pathname1 = d_path(&path, tmp, PAGE_SIZE);
        path_put(&path);
 	strcpy(pathname, pathname1);
        if(IS_ERR(pathname)){
                free_page((unsigned long)tmp);
                return pathname;
        }
       
        free_page((unsigned long)tmp);
       
        return;
}


// Take the same parameter as the system call WRITE
ssize_t hook_function(int fd, const void *buf, size_t count)
{
	// Print calling process name
	printk(KERN_INFO "Calling process:%s\n",current->comm);
	char pathname[255];
	fd_to_pathname(fd, pathname);
	printk(KERN_INFO "Written file: %s\n",pathname);
	int written_bytes = custom_syscall(fd, buf, count);
	printk(KERN_INFO "Number of written bytes:%d\n", written_bytes);
	return written_bytes;
}


static int __init entry_point(void){

	printk(KERN_INFO "Write Hook loaded successfully..\n");
	//system call table address
	system_call_table_addr = (void*)kallsyms_lookup_name("sys_call_table");
	// Assign custom_syscall to system call OPEN
	custom_syscall = system_call_table_addr[__NR_write];
	//Disable page protection
	make_rw((unsigned long)system_call_table_addr);
	// Replace system call OPEN by our Hook function
	system_call_table_addr[__NR_write] = hook_function;
	return 0;
}

static void __exit exit_point(void){
	printk(KERN_INFO "Unloaded Write Hook successfully\n");
	// Restore system call OPEN
	system_call_table_addr[__NR_write] = custom_syscall;
	//Renable page protection
	make_ro((unsigned long)system_call_table_addr);
	return;
}


module_init(entry_point);
module_exit(exit_point);
MODULE_LICENSE("GPL"); /* giay phep su dung cua module */
MODULE_AUTHOR("Quang Huy"); /* tac gia cua module */
MODULE_DESCRIPTION("hook-write for syscall Write"); /* mo ta chuc nang cua module */


