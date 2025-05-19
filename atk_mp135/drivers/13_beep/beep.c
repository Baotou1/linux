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
#define DEV_NAME                "beep"
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
}__udev;
__udev devbeep;
/****************************************************************************************/
/*文件接口操作函数*/
/* 打开设备 */
static int _open(struct inode *pinode,struct file *filp)
{
    filp->private_data = &devbeep;
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
/* 在设备树里查找对应硬件节点 */
static int fd_devnd_to_devtree(void)
{
    const char *str;
    int ret = 0;
/* 1.查找硬件节点 */
    devbeep.nd = of_find_node_by_name(NULL ,"beep");
    if(IS_ERR(devbeep.nd)){
        pr_err("find node to device-tree failed!!!\r\n");
        return -_HD_DEV_ND_ERROR;
    }
/* 2.读取硬件状态是否为okay */
    ret = of_property_read_string(devbeep.nd ,"status" ,&str);
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
    devbeep.ionum = of_get_named_gpio(devbeep.nd ,"gpios" ,0);
    if(devbeep.ionum < 0){
        pr_err("beep-gpio get ionum failed!!!\r\n");
        return -_HD_DEV_NUM_ERROR;
    }
    pr_notice("beep-gpio ionum = %d\r\n",devbeep.ionum);
/* 4.向gpio子系统申请使用gpio */
    ret = gpio_request(devbeep.ionum ,"beep-gpio");
    if(ret != 0){
        pr_err("beep-gpio request failed!!!\r\n");
        return -_HD_DEV_REQUEST_ERROR;
    }
/* 设置该Io口属性 该蜂鸣器输出低电平鸣叫，高电平不鸣叫*/
    ret = gpio_direction_output(devbeep.ionum ,1);
    if(ret < 0){
        pr_err("beep-gpio output set failed!!!\r\n");
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
    if(devbeep.major){
        /*得到dev_t的设备号*/
        devbeep.dev_id = MKDEV(devbeep.major ,0);
        if(devbeep.dev_id){
            pr_err("dev_id register failed!!!\r\n");
            return -DEV_ID_ERROR;
        }
    }
    else{
        /* 注册设备号 */
        ret = alloc_chrdev_region(&devbeep.dev_id ,0 ,DEV_CNT ,DEV_NAME);
        if(ret < 0){
            pr_err("dev_id register failed!!!\r\n");
            return -DEV_ID_ERROR;
        }
        devbeep.major = MAJOR(devbeep.dev_id);
        devbeep.minor = MINOR(devbeep.dev_id);
        pr_notice("%s: major = %d minor = %d \r\n",DEV_NAME,devbeep.major,devbeep.minor);
    }
/* 2.将设备注册到内核 */
    devbeep._cdev.owner = THIS_MODULE;
    /* 初始化cdev */
    cdev_init(&devbeep._cdev ,&_file_ops);
    /* 将cdev注册到内核 */
    ret = cdev_add(&devbeep._cdev ,devbeep.dev_id ,DEV_CNT);
    if(ret < 0){
        pr_err("cdev add kernel failed!!!\r\n");
        return -DEV_CDEV_ERROR;
    }
/* 3.分配类 */
    devbeep._class = class_create(devbeep._cdev.owner ,DEV_NAME);
    if(IS_ERR(devbeep._class)){
        pr_err("class create failed!!!\r\n");
        return -DEV_CLASS_ERROR;
    }
/* 4.创建设备层硬件 */
    devbeep._device = device_create(devbeep._class ,NULL ,devbeep.dev_id ,NULL ,DEV_NAME);
    if(IS_ERR(devbeep._device)){
        pr_err("device create failed!!!\r\n");
        return -DEV_DEVICE_ERROR; 
    }
    return 1;
}
/****************************************************************************************/
/*驱动加载函数*/
static int __init _dev_init(void)
{
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
    pr_notice("beep-gpio dev loaded successfully\r\n");
    return 0;
err_6:
    device_destroy(devbeep._class ,devbeep.dev_id);
err_5:
    class_destroy(devbeep._class);
err_4:
    cdev_del(&devbeep._cdev);
err_3:
    unregister_chrdev_region(devbeep.dev_id ,DEV_CNT);
err_2:
    gpio_free(devbeep.ionum);
err_1:
    return -EINVAL;
}
/*驱动卸载函数*/
static void __exit _dev_exit(void)
{
    device_destroy(devbeep._class ,devbeep.dev_id);
    class_destroy(devbeep._class);
    cdev_del(&devbeep._cdev);
    unregister_chrdev_region(devbeep.dev_id ,DEV_CNT);
    gpio_free(devbeep.ionum);
    pr_notice("%s logout finished\r\n",DEV_NAME);
}
/****************************************************************************************/
module_init(_dev_init);
module_exit(_dev_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("baotou");
MODULE_INFO(intree,"Y");