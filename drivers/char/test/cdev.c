#include <linux/init.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <asm/uaccess.h>
//1.定义cdev结构体
static struct cdev *cdev;
#define CDEVNAME "mycdev"
int major = 500;
int minor = 0;
int count = 3; 
struct class *cls;
struct device *device;

char kernel_buf[128] = {0};
int mycdev_open(struct inode *inode, struct file *filp)
{
	printk("%s:%s:%d\n",__FILE__,__func__,__LINE__);
	return 0;
}

ssize_t mycdev_read(struct file *filp, char __user * buf, size_t size, loff_t * offs)
{
	int ret;
	printk("%s:%s:%d\n",__FILE__,__func__,__LINE__);
	if(size > sizeof(kernel_buf)){
		size = sizeof(kernel_buf);	
	}

	ret = copy_to_user(buf,kernel_buf,size);
	if(ret){
		printk("copy data to user error.\n");
		return -EAGAIN;
	}

	return 0;
}

ssize_t mycdev_wirte(struct file *filp, const char __user * buf, size_t size, loff_t * offs)
{
	int ret;
	printk("%s:%s:%d\n",__FILE__,__func__,__LINE__);
	if(size > sizeof(kernel_buf)){
		size = sizeof(kernel_buf);	
	}

	ret = copy_from_user(kernel_buf,buf,size);
	if(ret){
		printk("copy data from user error.\n");
		return -EAGAIN;
	}
	printk("read data = %s.\n",kernel_buf);

	return 0;
}

int mycdev_close(struct inode *inode, struct file *filp)
{
	printk("%s:%s:%d\n",__FILE__,__func__,__LINE__);
	return 0;
}

static struct file_operations fops = {
	.open = mycdev_open,
	.read = mycdev_read,
	.write = mycdev_wirte,
	.release = mycdev_close,
};

static int __init mycdev_init(void)
{
	int ret,i;
	dev_t dev;
	//2.为cdev分配内存空间
	cdev = cdev_alloc();
	if(cdev == NULL){
		printk("alloc cdev memory fail.\n");
		ret = -ENOMEM;
		goto ERR_STEP1;
	}

	//3.cdev结构体的初始化
	cdev_init(cdev,&fops);

	//4.设备号的申请
	if(major > 0){
		ret = register_chrdev_region(MKDEV(major,minor),count,CDEVNAME);
		if(ret){
			printk("handle:alloc devnum error.\n");
			goto ERR_STEP1;
		}
	}else{
		ret = alloc_chrdev_region(&dev,0,count,CDEVNAME);
		if(ret){
			printk("dynamic:alloc devnum error.\n");
			goto ERR_STEP1;
		}
		major = MAJOR(dev);
		minor = MINOR(dev);
		printk("major = %d,minor = %d\n",major,minor);
	}	

	//5.注册字符设备驱动
	ret = cdev_add(cdev,MKDEV(major,minor),count);
	if(ret){
		printk("register chrdev error.\n");
		goto ERR_STEP2;
	}
	//6.创建目录
	cls = class_create(THIS_MODULE,CDEVNAME);
	if(IS_ERR(cls)){
		printk("class create error.\n");
		ret = PTR_ERR(cls);
		goto ERR_STEP3;
	}

	//7.设备文件的创建
	for(i=0; i<count; i++){
		device = device_create(cls,NULL,MKDEV(major,i),NULL,"mycdev%d",i);
		if(IS_ERR(device)){
			printk("device  %d create error",i);
			ret = PTR_ERR(device);
			goto ERR_STEP4;
		}
	}
	return 0;

ERR_STEP4:
	for(--i; i>=0; i--){
		device_destroy(cls,MKDEV(major,i));
	}

	class_destroy(cls);
ERR_STEP3:
	cdev_del(cdev);
ERR_STEP2:
	unregister_chrdev_region(MKDEV(major,minor),count);
ERR_STEP1:
	return ret;

}
static void __exit mycdev_exit(void)
{
	int i=0;
	//释放资源
	for(i=0; i<count; i++){
		device_destroy(cls,MKDEV(major,i));
	}
	class_destroy(cls);
	cdev_del(cdev);
	unregister_chrdev_region(MKDEV(major,minor),count);

}

module_init(mycdev_init);
module_exit(mycdev_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("farsight daizs_jt@hqyj.com");
