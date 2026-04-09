#include <linux/module.h>
#include <linux/init.h>
#include <linux/moduleparam.h>
#include <linux/version.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/string.h>
#include <linux/jiffies.h>
#include <linux/device.h>
#include <linux/wait.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ans Christy K L");
MODULE_DESCRIPTION("OS Assignment 1");

static dev_t dev_num;
static struct cdev my_cdev;

static struct class *my_class;
static struct device *my_device;

static char username[100];
static int username_written = 0;
static int read_done = 0;

static wait_queue_head_t my_wq;
static int timed_out = 0;
static long wait_ret;

static int order_ok = 0;

static int kernel_version[2];
static int kernel_version_count;
static int timer;

static unsigned long start_jiffies;

module_param_array(kernel_version, int, &kernel_version_count, 0);
module_param(timer, int, 0);

static char read_msg[] = "Hello from kernel device\n";
static ssize_t my_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
  printk(KERN_INFO "mymodule: read called\n");
  read_done = 1;

  wake_up_interruptible(&my_wq);

  size_t len = strlen(read_msg);
  if (*ppos >= len)
    return 0;
  if (count > len - *ppos)
    count = len - *ppos;
  if (copy_to_user(buf, read_msg + *ppos, count) != 0)
    return -EFAULT;

  *ppos += count;
  return count;
}

static ssize_t my_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
  printk(KERN_INFO "mymodule: write called\n");
  wait_ret = 1;

  if (!read_done)
  {
    printk(KERN_ERR "mymodule: waiting for read...\n");

    wait_ret = wait_event_interruptible_timeout(my_wq, read_done != 0, timer * HZ);
  }

  if (wait_ret == 0)
  {
    timed_out = 1;
    printk(KERN_ERR "mymodule: timeout happened before read\n");
    return -ETIMEDOUT;
  }

  if (wait_ret < 0)
  {
    printk(KERN_ERR "mymodule: wait was interrupted\n");
    return wait_ret;
  }

  if (count >= sizeof(username))
    count = sizeof(username) - 1;

  if (copy_from_user(username, buf, count) != 0)
    return -EFAULT;

  username[count] = '\0';
  username_written = 1;
  order_ok = 1;

  printk(KERN_INFO "mymodule: username stored = %s\n", username);

  return count;
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .read = my_read,
    .write = my_write,
};

static int __init my_init(void)
{
  int ret;
  int build_major, build_minor;
  printk(KERN_INFO "mymodule: loaded\n");

  build_major = (LINUX_VERSION_CODE >> 16) & 0xFF;
  build_minor = (LINUX_VERSION_CODE >> 8) & 0xFF;

  init_waitqueue_head(&my_wq);

  if (kernel_version_count != 2)
  {
    printk(KERN_ERR "Error: pass kernel_version as major,minor\n");
    return -EINVAL;
  }

  if (timer <= 0)
  {
    printk(KERN_ERR "Error: timer must be > 0\n");
    return -EINVAL;
  }

  printk(KERN_INFO "kernel_version_count = %d\n", kernel_version_count);

  if (kernel_version_count == 2)
  {
    printk(KERN_INFO "kernel_version = %d.%d\n", kernel_version[0], kernel_version[1]);
  }

  if (kernel_version[0] != build_major ||
      kernel_version[1] != build_minor)
  {

    printk(KERN_ERR "Error: kernel version mismatch\n");
    return -EINVAL;
  }

  printk(KERN_INFO "mymodule: version matched\n");
  printk(KERN_INFO "timer = %d seconds\n", timer);

  ret = alloc_chrdev_region(&dev_num, 0, 1, "mymodule");
  if (ret < 0)
  {
    printk(KERN_ERR "Error: failed to allocate device number\n");
    return ret;
  }

  printk(KERN_INFO "major = %d, minor = %d\n", MAJOR(dev_num), MINOR(dev_num));

  cdev_init(&my_cdev, &fops);
  my_cdev.owner = THIS_MODULE;

  ret = cdev_add(&my_cdev, dev_num, 1);
  if (ret < 0)
  {
    printk(KERN_ERR "Error: cdev_add failed\n");
    unregister_chrdev_region(dev_num, 1);
    return ret;
  }

  my_class = class_create("mymodule_class");
  if (IS_ERR(my_class))
  {
    cdev_del(&my_cdev);
    unregister_chrdev_region(dev_num, 1);
    return PTR_ERR(my_class);
  }

  my_device = device_create(my_class, NULL, dev_num, NULL, "mymodule");
  if (IS_ERR(my_device))
  {
    class_destroy(my_class);
    cdev_del(&my_cdev);
    unregister_chrdev_region(dev_num, 1);
    return PTR_ERR(my_device);
  }

  start_jiffies = jiffies;
  return 0;
}

static void __exit my_exit(void)
{
  unsigned long elapsed_jiffies;
  unsigned long elapsed_seconds;

  elapsed_jiffies = jiffies - start_jiffies;
  elapsed_seconds = elapsed_jiffies / HZ;

  printk(KERN_INFO "mymodule: read_done = %d, username_written = %d, order_ok = %d\n", read_done, username_written, order_ok);

  if (username_written)
    printk(KERN_INFO "mymodule: stored username = %s\n", username);

  printk(KERN_INFO "mymodule: elapsed time = %lu seconds\n", elapsed_seconds);

  if (!timed_out && read_done && username_written && order_ok && elapsed_seconds <= timer)
  {
    printk(KERN_INFO "Successfully completed the actions within time. Username: %s\n", username);
  }
  else
  {
    printk(KERN_ERR "Failed to complete the actions correctly within time\n");
  }

  device_destroy(my_class, dev_num);
  class_destroy(my_class);
  cdev_del(&my_cdev);
  unregister_chrdev_region(dev_num, 1);
  printk(KERN_INFO "mymodule: removed\n");
}

module_init(my_init);
module_exit(my_exit);
