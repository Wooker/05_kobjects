#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/uaccess.h>
#include <linux/fs.h>

#define DEVICE_NAME "solution_node"
#define SOLUTION_MAJOR 217

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
  printk(KERN_INFO "Reading sum %d", sum);
  return sprintf(buf, "%d\n", sum);
}
 
static ssize_t my_sys_store(struct kobject* kobj, struct kobj_attribute* attr, const char* buf, size_t count)
{
    return 0;
}
 
static struct kobject* my_kobject;
static struct kobj_attribute my_sys_attribute = __ATTR(my_sys, 0664, my_sys_show, my_sys_store);

static ssize_t solution_read(struct file *filp, char __user *buf, size_t count, loff_t *offset)
{
  int result; 
  char sum_string[15]; // Allocate a buffer to hold the sum as a string
  int len = snprintf(sum_string, sizeof(sum_string), "%d\n", sum); // Convert sum to string

  if (*offset >= len)
      return 0; // End of file


  result = copy_to_user(buf, sum_string, len); 
  if ( result != 0)
    return -EFAULT; // Failed to copy sum to user space

  *offset += len;
  return len;
}

static ssize_t solution_write(struct file *filp, const char __user *buf, size_t count, loff_t *offset)
{
    int result;
    char input_buffer[24]; // Allocate a buffer to store the user input

    result = copy_from_user(input_buffer, buf, count); 

    if ( result != 0)
        return -EFAULT; // Failed to copy from user space

    if (sscanf(input_buffer, "%d %d", &a, &b) != 2)
        return -EINVAL; // Invalid input format

  int i;
  for (i = 0; i < 5; i++) {
    if (sscanf(input_buffer, "%d", &c[i]) != 1)
        return -EINVAL; // Invalid input format
    
  }

    return count;
}

// Register operations
static struct file_operations solution_fops = {
    .owner = THIS_MODULE,
    .read = solution_read,
    .write = solution_write,
};

static int __init solution_init(void)
{
    int result, i;
    sum += a + b;
    for (i = 0; i < 5; i++) {
      printk(KERN_INFO "Adding to sum %d", c[i]);
      sum += c[i];
    }

    // Register the character device driver
    result = register_chrdev(SOLUTION_MAJOR, DEVICE_NAME, &solution_fops);
    if (result < 0) {
        printk(KERN_ALERT "Failed to register solution module %d\n", result);
        return result;
    }

    my_kobject = kobject_create_and_add("my_kobject", kernel_kobj);
    if (!my_kobject)
    {
      printk(KERN_ERR "kernel_mooc error kobject_create_and_add");
      return 1;
    }
 
    result = sysfs_create_file(my_kobject, &my_sys_attribute.attr);
    if (result )
    {
      printk(KERN_ERR "kernel_mooc error sysfs_create_file");
      kobject_put(my_kobject);
      return 1;
    }

    printk(KERN_INFO "Solution module is loaded\n");
    return 0;
}

static void __exit solution_exit(void)
{
    // Unregister the character device driver
    unregister_chrdev(SOLUTION_MAJOR, DEVICE_NAME);

    printk(KERN_INFO "Solution module is unloaded\n");
}

module_init(solution_init);
module_exit(solution_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Zakhar Semenov");
MODULE_DESCRIPTION("Solution");
