#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/random.h>
#include <linux/uaccess.h>

static dev_t first; // Global variable for the first device number
static struct cdev c_dev; // Global variable for the character device structure
static struct class* cl; // Global variable for the device class
static int my_open(struct inode* i, struct file* f)
{
	printk(KERN_INFO "Driver: open()\n");
	return 0;
}
static int my_close(struct inode* i, struct file* f)
{
	printk(KERN_INFO "Driver: close()\n");
	return 0;
}

int my_random(void)
{
	int i;
	get_random_bytes(&i,sizeof(i));
	return i;
}

static char c;
static ssize_t my_read(struct file* f, char __user* buf, size_t
	len, loff_t *off)
{
	int i = my_random();
	sprintf(buf,"%d",i);
	printk(KERN_INFO "Driver: read()\n");
	/* You have just a single char in your buffer, so only 0 offset is valid */
	if(*off > 0)
    	 return 0; /* End of file */
	if (copy_to_user(buf, &c, 1))
	 return -EFAULT;
	*off++;
	return 1;
}

static ssize_t my_write(struct file* f, const char __user* buf,
    size_t len, loff_t* off)
{
	printk(KERN_INFO "Driver: write()\n");
	if (copy_from_user(&c, buf + len - 1, 1) != 0)
	 return -EFAULT;
 	else
	 return len;
	
}

static struct file_operations pugs_fops =
{
	.owner = THIS_MODULE,
	.open = my_open,
	.release = my_close,
	.read = my_read,
	.write = my_write
};

static int __init ofcd_init(void) /* Constructor */
{
	printk(KERN_INFO "Hello: ofcd registered");
	if (alloc_chrdev_region(&first, 0, 1, "Shweta") < 0)
	{
		return -1;
	}
	if ((cl = class_create(THIS_MODULE, "chardrv")) == NULL)
	{
		unregister_chrdev_region(first, 1);
		return -1;
	}
	if (device_create(cl, NULL, first, NULL, "mynull") == NULL)
	{
		class_destroy(cl);
		unregister_chrdev_region(first, 1);
		return -1;
	}
	cdev_init(&c_dev, &pugs_fops);
	if (cdev_add(&c_dev, first, 1) == -1)
	{
		device_destroy(cl, first);
		class_destroy(cl);
		unregister_chrdev_region(first, 1);
		return -1;
	}
	return 0;
}
static void __exit ofcd_exit(void) /* Destructor */
{
	cdev_del(&c_dev);
	device_destroy(cl, first);
	class_destroy(cl);
	unregister_chrdev_region(first, 1);
	printk(KERN_INFO "Alvida: ofcd unregistered");
}


module_init(ofcd_init);
module_exit(ofcd_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Chau Vinh Lap");
MODULE_DESCRIPTION("Our First Character Driver");
