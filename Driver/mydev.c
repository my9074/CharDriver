#include <linux/module.h>   
#include <linux/types.h>   
#include <linux/fs.h>   
#include <linux/errno.h>   
#include <linux/mm.h>   
#include <linux/sched.h>   
#include <linux/init.h>   
#include <linux/cdev.h>   
#include <asm/io.h>   
#include <asm/system.h>   
#include <asm/uaccess.h>
#include <linux/slab.h>                  //kmalloc  kfree
#include <linux/kernel.h>
#include <linux/major.h>
#include <linux/device.h>
//#define MYDEV_SIZE 0x2000 
//#define MEM_CLEAR 0x1  
#define MYDEV_MAJOR 0
#define MYDEV_SIZE 0x1000
#define MYDEV_NAME  "mydev"

static dev_t devno;

//static unsigned char mem[MYDEV_SIZE];

static struct class *mydev_class;

static int mydev_major = MYDEV_MAJOR;    /*之前错过 之前没定义mydev_major*/

struct mydev_dev{
    struct cdev cdev;    
    unsigned char mem[MYDEV_SIZE];          
};

struct mydev_dev *mydev_devp;

int mydev_open(struct inode *inode, struct file *filp){
    printk("mydev open.\n");
    filp->private_data = mydev_devp; 
    return 0;

}

int mydev_release(struct inode *inode, struct file *filp){
  printk("mydev has been close.\n");
  return 0;

}

static ssize_t mydev_read(struct file *filp, char __user *buf, size_t size, loff_t *ppos)

{
  unsigned long p =  *ppos;
  unsigned long count = size;  
  int ret = 0;  
  struct mydev_dev *devp = filp->private_data; 		/*之前错过 应定义成mydev_dev结构体不是mydev */
	printk("mydev_read.\n");
	

	
  if (p >= MYDEV_SIZE)  
    return 0;
  if (count > MYDEV_SIZE - p)
    count = MYDEV_SIZE - p;
   // memset(mem,0,sizeof(mem));
  if (copy_to_user(buf, (void*)(devp->mem + p), count)){
    ret =  - EFAULT;
  }else{
    *ppos += count;
    ret = count;
    printk(KERN_INFO "read %lu bytes(s) from %lu\n", count, p);
  }
	
	memset(&devp->mem,0,sizeof(devp->mem));
		
  return ret;

}

static loff_t mydev_llseek(struct file *filp,loff_t offset,int orig){
	loff_t ret;
	switch(orig){
		case 0:
					if(offset<0){
							ret = -EINVAL;
							break;
						}
					if((unsigned int)offset > MYDEV_SIZE){
							ret = -EINVAL;
							break;
						}
			filp->f_pos = (unsigned int)offset;
			ret = filp->f_pos;
			printk("Enter the area form 0\n");
			break;
		case 1:
					if((filp->f_pos + offset) > MYDEV_SIZE){
							ret = -EINVAL;
							break;
						}
					if((filp->f_pos + offset) < 0){
							ret = - EINVAL;
							break;
						}
			filp->f_pos += offset;
			ret = filp->f_pos;
			printk("Enter the area form 1\n");
			break;
		default:
					ret = -EINVAL;
		}
		return ret;
}

static ssize_t mydev_write(struct file *filp, const char __user *buf, size_t size, loff_t *ppos){
  unsigned long p =  *ppos; 
  int ret = 0; 
  unsigned long count = size;
    
  struct mydev_dev*devp = filp->private_data; 		/*之前错过 应定义成mydev_dev结构体不是mydev */
	printk("mydev_write.\n");
  if (p >= MYDEV_SIZE)
	return 0;
  if (count > MYDEV_SIZE - p)
    count = MYDEV_SIZE - p;
  if (copy_from_user(devp->mem + p, buf, count))
    ret =  - EFAULT;
  else{
    *ppos += count;
    ret = count;
    printk(KERN_INFO "written %lu bytes(s) from %lu\n", count, p);		/*注意格式符号和 69 71行定义的对应*/
  }
  return ret;
}


static const struct file_operations mydev_fops ={
  .owner = THIS_MODULE,
  .read = mydev_read,    
  .write = mydev_write,     
  .open = mydev_open,     
  .release = mydev_release,
  .llseek = mydev_llseek,

};

static void mydev_setup_cdev(struct mydev_dev *dev, int minor){				/*第一个参数不是mydev*/

    int err;
    dev_t devno = MKDEV(mydev_major, minor);   
    cdev_init(&dev->cdev, &mydev_fops); 
    dev->cdev.owner = THIS_MODULE;         
    dev->cdev.ops = &mydev_fops;     
    err = cdev_add(&dev->cdev, devno, 1);
    if (err)
       	printk(KERN_NOTICE "Error in cdev_add()\n");

}

static int mydev_init(void){
	int result;
	//dev_t devno =MKDEV(mydev_major,0);

	if(mydev_major == 0){
		result = alloc_chrdev_region(&devno,0,1,MYDEV_NAME);
		mydev_major = MAJOR(devno);
	}
	else{
		dev_t devno =MKDEV(mydev_major,0);
		result = register_chrdev_region(devno,1,MYDEV_NAME);
}
	if(result<0)
		return result;
	mydev_devp = kmalloc(sizeof(struct mydev_dev), GFP_KERNEL);
	if(!mydev_devp){
		result = -ENOMEM;
		goto fail_malloc;
	}
	memset(mydev_devp, 0, sizeof(struct mydev_dev));
	
	mydev_setup_cdev(mydev_devp, 0);

	mydev_class = class_create(THIS_MODULE, MYDEV_NAME);
	device_create(mydev_class,NULL,MKDEV(mydev_major,MINOR(devno)),NULL, MYDEV_NAME);
	
	return 0;
	
	fail_malloc:
		unregister_chrdev_region(devno,1);
		return result;
}



static void mydev_exit(void){
	cdev_del(&mydev_devp->cdev);
	kfree(mydev_devp);
	unregister_chrdev_region(MKDEV(mydev_major,0),1);
}


module_init(mydev_init);
module_exit(mydev_exit);

 

MODULE_AUTHOR("myu <myu@gmail.com>");
MODULE_LICENSE("GPL");

