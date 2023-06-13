#include <linux/kernel.h>
#include <linux/kobject.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/string.h>
#include <linux/sysfs.h>
#include <linux/fs.h>

#define KBUF_SIZE (size_t) ((10) * PAGE_SIZE)
 
static int a = 0;
module_param(a,int,0);
MODULE_PARM_DESC(a, "First integer");

static int b = 0;
module_param(b,int,0);
MODULE_PARM_DESC(b, "Second integer");

int c[5];
static int arr_argc = 0;
module_param_array(c, int, &arr_argc, 0);
MODULE_PARM_DESC(myintArray, "An array of integers");

static int sum = 0;
 
static ssize_t my_sys_show(struct kobject* kobj, struct kobj_attribute* attr, char* buf)
{
  buf[0] = '\0';
  return sprintf(buf, "%d\n", sum);
}
 
static ssize_t my_sys_store(struct kobject* kobj, struct kobj_attribute* attr, const char* buf, size_t count)
{
    return 0;
}
 
static struct kobject* my_kobject;
static struct kobj_attribute my_sys_attribute = __ATTR(my_sys, 0664, my_sys_show, my_sys_store);

static int drv_open(struct inode *inode, struct file *file) {
  char *kbuf = kmalloc(KBUF_SIZE, GFP_KERNEL);
  file->private_data = kbuf;

  return 0;
}

static int drv_read(struct file *file, char __user *buf, size_t lbuf, loff_t *ppos) {
  char *kbuf = file->private_data;

  int nbytes = lbuf - (int)copy_to_user(buf, kbuf + *ppos, lbuf);
  *ppos += nbytes;

  return nbytes;
}

static ssize_t drv_write(struct file *file, const char __user *buf, size_t lbuf, loff_t *ppos) {
  char *kbuf = file->private_data;

  int nbytes = (int)copy_from_user(kbuf + *ppos, buf, lbuf);
  *ppos += nbytes;

  printk(KERN_INFO "wrote %d bytes, %d ppos", nbytes, (int)*ppos);

  return nbytes;
}
 
static const struct file_operations my_fops = {
  .owner = THIS_MODULE,
  .open = drv_open,
  .write = drv_write,
};

static int __init custom_init(void)
{
  int retval = 0;
  printk(KERN_INFO "kernel_mooc Module initialize");
  
  my_kobject = kobject_create_and_add("my_kobject", kernel_kobj);
  if (!my_kobject)
  {
    printk(KERN_ERR "kernel_mooc error kobject_create_and_add");
    return 1;
  }
 
  retval = sysfs_create_file(my_kobject, &my_sys_attribute.attr);
  if (retval)
  {
    printk(KERN_ERR "kernel_mooc error sysfs_create_file");
    kobject_put(my_kobject);
    return 1;
  }
  
  return 0;
}
 
static void __exit custom_exit(void)
{
  printk(KERN_INFO "kernel_mooc Module deinitialize");
  kobject_put(my_kobject);
}
 
module_init(custom_init);
module_exit(custom_exit);
 
MODULE_LICENSE("GPL");