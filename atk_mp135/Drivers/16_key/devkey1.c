#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/gpio.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/kdev_t.h>
#include <asm/mach/map.h>
#include <asm/uaccess.h>
#include <linux/of_gpio.h>
#include <asm/io.h>
/****************************************************************************************/
#define DEV_NAME                "key1-pf14"
#define DEV_CNT                 0x01
#define KEY1VALUE		        0XF0	/* 按键值 		*/
#define INVAKEY			        0X00	/* 无效的按键值  */
 

#define _HD_DEV_ND_ERROR        0x01
#define _HD_DEV_STA_ERROR       0x02
#define _HD_DEV_NUM_ERROR       0x03
#define _HD_DEV_REQUEST_ERROR   0x04
#define _HD_DEV_OUTPUT_ERROR    0x05
#define DEV_ID_ERROR            0x01
#define DEV_CDEV_ERROR          0x02
#define DEV_cls_ERROR           0x03
#define DEV_dev_ERROR           0x04
/****************************************************************************************/
typedef struct{
    int major;            
    int minor;            
    dev_t dev_id;          
    struct cdev _cdev;     
    struct class *_cls;     
    struct device *_dev; 
    struct device_node *nd; 
    int io;            
    atomic_t a_val;         /* 按键原子操作变量值 */
}__udev;
__udev devkey1;
/****************************************************************************************/
/*文件接口操作函数*/
/* 打开设备 */
static int _open(struct inode *pinode,struct file *filp)
{
    filp->private_data = &devkey1;
    return 0;
}
/* 读取设备 */
static ssize_t _read(struct file *filp,char __user *buf,size_t cnt,loff_t* offt)
{
    int ret;
    __udev* _dev = filp->private_data;
    if(cnt == 0){
        pr_err("cnt error\r\n");
        return -ENAVAIL;
    }
    /* 假设key1未被按下 */
    atomic_set(&_dev->a_val ,INVAKEY);
    /* 读取驱动devkey1所对应的Io口电平 */
    /* key1按下 */
    if(gpio_get_value(_dev->io) == 0){
        /* 消抖 */
        msleep(20);
        while(!gpio_get_value(_dev->io)){
            atomic_set(&_dev->a_val ,KEY1VALUE);
        }
    }
    ret = copy_to_user(buf ,&_dev->a_val ,cnt);
    if(ret < 0){
        pr_err("copy to user error\r\n");
        return -ENAVAIL;
    }
    return 0;
}
/* 写入设备 */
static ssize_t _write(struct file *filp,const char __user *buf,size_t cnt,loff_t* offt)
{
    return 0;
}
/* 关闭设备 */
static int _release(struct inode *pinode,struct file *filp)
{
    return 0;
}

static struct file_operations _file_ops = {
    .owner = THIS_MODULE,/* 确保在设备文件被操作时，内核模块不会被卸载 */
    .open = _open,
    .read = _read,
    .write = _write,
    .release = _release
};
/****************************************************************************************/
/* 初始化原子变量 */
static void _syncp_init(void)
{
    devkey1.a_val = (atomic_t)ATOMIC_INIT(0);
    atomic_set(&devkey1.a_val ,INVAKEY);
    pr_notice("syncp_ato init successd!!!\r\n");
}
/* 在设备树里查找对应硬件节点 */
static int fd_devnd_to_devtree(void)
{
    const char *str;
    int ret = 0;
/* 通过节点名字查找硬件节点 */
    devkey1.nd = of_find_node_by_name(NULL ,"key1");
    if(IS_ERR(devkey1.nd)){
        pr_err("find node to device-tree failed!!!\r\n");
        return -_HD_DEV_ND_ERROR;
    }
/* 读取其中任意一个属性看是否匹配 */
    ret = of_property_read_string(devkey1.nd ,"label" ,&str);
    if(ret != 0){
        pr_err("read srting label error\r\n");
        return -_HD_DEV_STA_ERROR;
    }
    if(strcmp(str ,"USER-KEY1") != 0){
        pr_err("read srting is not USER-KEY1\r\n");
        return -_HD_DEV_STA_ERROR;
    }
/* 通过gpio子系统信息，获取PIN编号 */
    devkey1.io = of_get_named_gpio(devkey1.nd ,"gpios" ,0);
    if(devkey1.io < 0){
        pr_err("key1-gpio get io failed!!!\r\n");
        return -_HD_DEV_NUM_ERROR;
    }
    pr_notice("key1-gpio io = %d\r\n",devkey1.io);
/* 向gpio子系统申请使用gpio */
    ret = gpio_request(devkey1.io ,"key1-gpio");
    if(ret != 0){
        pr_err("key1-gpio request failed!!!\r\n");
        return -_HD_DEV_REQUEST_ERROR;
    }
/* 设置该Io口属性为输入模式 */
    ret = gpio_direction_input(devkey1.io);
    if(ret < 0){
        pr_err("key1-gpio input set failed!!!\r\n");
        return -_HD_DEV_OUTPUT_ERROR;
    }
    return 1;
}

/* 将硬件设备注册到内核 */
static int _dev_regsiter_kerenl(void)
{
    int ret = 0;
/* 1.创建设备号 */
    /*主设备号已经定义*/
    if(devkey1.major){
        /*得到dev_t的设备号*/
        devkey1.dev_id = MKDEV(devkey1.major ,0);
        if(devkey1.dev_id){
            pr_err("dev_id register failed!!!\r\n");
            return -DEV_ID_ERROR;
        }
    }
    else{
        /* 注册设备号 */
        ret = alloc_chrdev_region(&devkey1.dev_id ,0 ,DEV_CNT ,DEV_NAME);
        if(ret < 0){
            pr_err("dev_id register failed!!!\r\n");
            return -DEV_ID_ERROR;
        }
        devkey1.major = MAJOR(devkey1.dev_id);
        devkey1.minor = MINOR(devkey1.dev_id);
        pr_notice("%s: major = %d minor = %d \r\n",DEV_NAME,devkey1.major,devkey1.minor);
    }
/* 2.将设备注册到内核 */
    devkey1._cdev.owner = THIS_MODULE;
    /* 初始化cdev */
    cdev_init(&devkey1._cdev ,&_file_ops);
    /* 将cdev注册到内核 */
    ret = cdev_add(&devkey1._cdev ,devkey1.dev_id ,DEV_CNT);
    if(ret < 0){
        pr_err("cdev add kernel failed!!!\r\n");
        return -DEV_CDEV_ERROR;
    }
/* 3.分配类 */
    devkey1._cls = class_create(devkey1._cdev.owner ,DEV_NAME);
    if(IS_ERR(devkey1._cls)){
        pr_err("class create failed!!!\r\n");
        return -DEV_cls_ERROR;
    }
/* 4.创建设备层硬件 */
    devkey1._dev = device_create(devkey1._cls ,NULL ,devkey1.dev_id ,NULL ,DEV_NAME);
    if(IS_ERR(devkey1._dev)){
        pr_err("device create failed!!!\r\n");
        return -DEV_dev_ERROR; 
    }
    return 1;
}
/****************************************************************************************/
/*驱动加载函数*/
static int __init _dev_init(void)
{
    _syncp_init();
    switch(fd_devnd_to_devtree()){
        case -_HD_DEV_ND_ERROR:
            goto err_1;
        case -_HD_DEV_STA_ERROR:
            goto err_1;   
        case -_HD_DEV_NUM_ERROR:
            goto err_1;
        case -_HD_DEV_REQUEST_ERROR:
            goto err_1;
        case -_HD_DEV_OUTPUT_ERROR:
            goto err_2;    
    }
    switch(_dev_regsiter_kerenl()){
        case -DEV_ID_ERROR:
            goto err_3;
        case -DEV_CDEV_ERROR:
            goto err_4;
        case -DEV_cls_ERROR:
            goto err_5;
        case -DEV_dev_ERROR:
            goto err_6;
    }
    pr_notice("beep-gpio dev loaded successfully\r\n");
    return 0;
err_6:
    device_destroy(devkey1._cls ,devkey1.dev_id);
err_5:
    class_destroy(devkey1._cls);
err_4:
    cdev_del(&devkey1._cdev);
err_3:
    unregister_chrdev_region(devkey1.dev_id ,DEV_CNT);
err_2:
    gpio_free(devkey1.io);
err_1:
    return -EINVAL;
}
/*驱动卸载函数*/
static void __exit _dev_exit(void)
{
    device_destroy(devkey1._cls ,devkey1.dev_id);
    class_destroy(devkey1._cls);
    cdev_del(&devkey1._cdev);
    unregister_chrdev_region(devkey1.dev_id ,DEV_CNT);
    gpio_free(devkey1.io);
    pr_notice("%s Unload Finished\r\n",DEV_NAME);
}
/****************************************************************************************/
module_init(_dev_init);
module_exit(_dev_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("baotou");
MODULE_INFO(intree,"Y");