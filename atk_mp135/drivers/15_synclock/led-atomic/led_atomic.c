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
#define DEV_NAME                "led-pi3"
#define DEV_CNT                 0x01

#define _HD_DEV_ND_ERROR        0x01
#define _HD_DEV_STA_ERROR       0x02
#define _HD_DEV_NUM_ERROR       0x03
#define _HD_DEV_REQUEST_ERROR   0x04
#define _HD_DEV_OUTPUT_ERROR    0x05
#define DEV_ID_ERROR            0x01
#define DEV_CDEV_ERROR          0x02
#define DEV_CLASS_ERROR         0x03
#define DEV_DEVICE_ERROR        0x04
/****************************************************************************************/
typedef struct{
    int major;              /* 主设备号 */
    int minor;              /* 次设备号 */
    dev_t dev_id;           /* 设备号:内核通过设备号识别设备，调用驱动程序 */
    struct cdev _cdev;      /* 驱动层
                               管理字符设备，向内核注册设备驱动，让内核识别设备
                               含有俩个重要成员：dev_t（将设备id号与该结构体设备号绑定）
                               和file_operations与创建的file_operations绑定（提供open/write/read/release函数）*/
    struct class *_class;   /* 中间桥梁：分类设备，挂到(/sys/class/)
                               它是 “一类设备” 的名字，像LED类、字符设备、网络设备、USB 等 */
    struct device *_device; /* 设备层：创建设备节点
                               代表一个真实设备实例,比如LED1
                               配合 class，为用户程序创建设备节点(/dev/xx节点) */
    struct device_node *nd; /* 读取设备树中的节点，申请使用硬件设备，设备树节点：描绘硬件信息 */
    int ionum;              /* io编号：通常通过引脚编号来指定 GPIO */
    atomic_t _aatom;        /* 原子变量 */
}__udev;
__udev devled;
/****************************************************************************************/
/*文件接口操作函数*/
/* 打开设备 */
static int _open(struct inode *pinode,struct file *filp)
{
    /* 获取原子变量 */
    if(!atomic_dec_and_test(&devled._aatom)){
        /* 设备正在使用，将减掉的1再加回来 */
        atomic_inc(&devled._aatom);
        pr_err("\r\n");
        pr_err("led-pi3 busy!!!\r\n");
        return -1;
    }
    filp->private_data = &devled;
    return 0;
}
/* 读取设备 */
static ssize_t _read(struct file *filp,char __user *buf,size_t cnt,loff_t* offt)
{
    int ret;
    u8 sta;
    __udev *_udev = filp->private_data;
    if(cnt < 1)
        return -1;
    sta = gpio_get_value(_udev->ionum);
    ret = copy_to_user(buf ,&sta ,1);
    if(ret < 0)
        return -1;
    return 0;
}
/* 写入设备 */
static ssize_t _write(struct file *filp,const char __user *buf,size_t cnt,loff_t* offt)
{
    int ret;
    u8 wdata;
    __udev *_udev = filp->private_data;
    if(cnt < 1)
        return -1;
    ret = copy_from_user(&wdata ,buf ,1);
    if(ret < 0)
        return -1;
    gpio_set_value(_udev->ionum ,wdata);
    return 0;
}
/* 关闭设备 */
static int _release(struct inode *pinode,struct file *filp)
{
    __udev *_udev = filp->private_data;
    /* 释放原子变量 */
    atomic_inc(&_udev->_aatom);
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
/* 初始化同步量 */
static void _syncp_init(void)
{   
    devled._aatom = (atomic_t)ATOMIC_INIT(0);
    atomic_set(&devled._aatom ,1);
    pr_notice("syncp init successd!!!\r\n");
}
/* 在设备树里查找对应硬件节点 */
static int fd_devnd_to_devtree(void)
{
    const char *str;
    int ret = 0;
/* 1.查找硬件节点 */
    devled.nd = of_find_node_by_name(NULL ,"led-red");
    if(IS_ERR(devled.nd)){
        pr_err("find node to device-tree failed!!!\r\n");
        return -_HD_DEV_ND_ERROR;
    }
/* 2.读取硬件状态是否为okay */
    ret = of_property_read_string(devled.nd ,"status" ,&str);
    if(ret < 0){
        pr_err("status find error!!!\r\n");
        return -_HD_DEV_STA_ERROR;
    }
    ret = strcmp(str ,"okay");
    if(ret != 0){
        pr_err("status not is okay!!!\r\n");
        return -_HD_DEV_STA_ERROR;
    }
/* 3.通过gpio子系统信息，获取PIN编号 */
    devled.ionum = of_get_named_gpio(devled.nd ,"gpios" ,0);
    if(devled.ionum < 0){
        pr_err("led-gpio get ionum failed!!!\r\n");
        return -_HD_DEV_NUM_ERROR;
    }
    pr_notice("led-gpio ionum = %d\r\n",devled.ionum);
/* 4.向gpio子系统申请使用gpio */
    ret = gpio_request(devled.ionum ,"led-gpio");
    if(ret != 0){
        pr_err("led-gpio request failed!!!\r\n");
        return -_HD_DEV_REQUEST_ERROR;
    }

    ret = gpio_direction_output(devled.ionum ,1);
    if(ret < 0){
        pr_err("led-gpio output set failed!!!\r\n");
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
    if(devled.major){
        /*得到dev_t的设备号*/
        devled.dev_id = MKDEV(devled.major ,0);
        if(devled.dev_id){
            pr_err("dev_id register failed!!!\r\n");
            return -DEV_ID_ERROR;
        }
    }
    else{
        /* 注册设备号 */
        ret = alloc_chrdev_region(&devled.dev_id ,0 ,DEV_CNT ,DEV_NAME);
        if(ret < 0){
            pr_err("dev_id register failed!!!\r\n");
            return -DEV_ID_ERROR;
        }
        devled.major = MAJOR(devled.dev_id);
        devled.minor = MINOR(devled.dev_id);
        pr_notice("%s: major = %d minor = %d \r\n",DEV_NAME,devled.major,devled.minor);
    }
/* 2.将设备注册到内核 */
    devled._cdev.owner = THIS_MODULE;
    /* 初始化cdev */
    cdev_init(&devled._cdev ,&_file_ops);
    /* 将cdev注册到内核 */
    ret = cdev_add(&devled._cdev ,devled.dev_id ,DEV_CNT);
    if(ret < 0){
        pr_err("cdev add kernel failed!!!\r\n");
        return -DEV_CDEV_ERROR;
    }
/* 3.分配类 */
    devled._class = class_create(devled._cdev.owner ,DEV_NAME);
    if(IS_ERR(devled._class)){
        pr_err("class create failed!!!\r\n");
        return -DEV_CLASS_ERROR;
    }
/* 4.创建设备层硬件 */
    devled._device = device_create(devled._class ,NULL ,devled.dev_id ,NULL ,DEV_NAME);
    if(IS_ERR(devled._device)){
        pr_err("device create failed!!!\r\n");
        return -DEV_DEVICE_ERROR; 
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
        case -DEV_CLASS_ERROR:
            goto err_5;
        case -DEV_DEVICE_ERROR:
            goto err_6;
    }
    pr_notice("led-gpio dev loaded successfully\r\n");
    return 0;
err_6:
    device_destroy(devled._class ,devled.dev_id);
err_5:
    class_destroy(devled._class);
err_4:
    cdev_del(&devled._cdev);
err_3:
    unregister_chrdev_region(devled.dev_id ,DEV_CNT);
err_2:
    gpio_free(devled.ionum);
err_1:
    return -EINVAL;
}
/*驱动卸载函数*/
static void __exit _dev_exit(void)
{
    device_destroy(devled._class ,devled.dev_id);
    class_destroy(devled._class);
    cdev_del(&devled._cdev);
    unregister_chrdev_region(devled.dev_id ,DEV_CNT);
    gpio_free(devled.ionum);
    pr_notice("%s logout finished\r\n",DEV_NAME);
}
/****************************************************************************************/
module_init(_dev_init);
module_exit(_dev_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("baotou");
MODULE_INFO(intree,"Y");
