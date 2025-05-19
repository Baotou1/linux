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
/*******************************************************************
Copyright © ALIENTEK Co., Ltd. 1998-2029. All rights reserved.
文件名 : led.c
作者 : baotou
版本 : V1.0
描述 : LED 驱动文件。
其他 : 无
日志 : 初版 V1.0 2025/3/20 
********************************************************************/
/*宏定义************************************************************/
#define GPIOLED_CNT                 (0x01)
#define GPIOLED_NAME                ("gpioled")
/*变量*************************************************************/
/* gpioled_dev设备结构体 */
struct gpioled_dev
{
	dev_t devid;			/* 设备id */
	struct cdev _cdev;      /* 设备信息 */
	struct class *_class;   /* 设备类 */
	struct device *_device; /* 设备 */
	int major;              /* 主设备号 */
	int minor;              /* 次设备号 */
    struct device_node *nd; /* 设备节点 */
    int num;                /* led的gpio编号 */
};
struct gpioled_dev _gpioled;  /* led设备 */
/******************************************************************/
/*
* @function_name : GPIOx_xio_open
* @description   : 打开具体引脚
* @param  pinode : 传递给驱动的 inode(节点，在文件创建时创建，包含了文件的元数据)
* @param  pfilp  : 设备文件， file 结构体有个叫做 private_data
*                  (用于存储文件的私有数据，由文件操作函数使用)的成员变量
*                  一般在 open 的时候将 private_data 指向设备结构体。
* @return        : 0 
*/
static int GPIOx_xio_open(struct inode *pinode ,struct file *pfile)
{ 
    /* 将设备结构体指针存储在 private_data中 */
	pfile->private_data = &_gpioled;
    return 0;    
}
/*
* @function_name : GPIOx_xio_read
* @description   : 从设备读取数据
* @param - pfilp : 要打开的设备文件(文件描述符)
* @param - buf   : 返回给用户空间的数据缓冲区
* @param - cnt   : 要读取的数据长度
* @param - offt  : 相对于文件首地址的偏移
* @return        : 0 -- 成功；-1 -- 失败
*/
static ssize_t GPIOx_xio_read(struct file *pfile ,char __user *buf ,size_t cnt ,loff_t *offt)
{
    int ret = 0;
    u8 io_sta = 0;
    /* 获取_gpioled数据结构，在这里可以通过dev访问设备的GPIO控制或LED状态等 */
    struct gpioled_dev *dev = pfile->private_data;
    if(cnt < 1){
        return -1;
    }

    /* 读取io口 */
    io_sta = gpio_get_value(dev->num);
    if(cnt == 1)
    {
        ret = copy_to_user(buf ,&io_sta ,1);
        /* buf 是用户空间指针，不能直接解引用 *buf，否则会导致内核崩溃 */
        pr_notice("io_sta == %d\r\n", io_sta);
        if(ret < 0){
            return -1;
        }
    }
    return 0;
}
/*
* @function_name : GPIOx_xio_write
* @description   : 向设备写数据
* @param - filp  : 设备文件，表示打开的文件描述符
* @param - buf   : 要写给设备写入的数据
* @param - cnt   : 要写入的数据长度
* @param - offt  : 相对于文件首地址的偏移
* @return        : 0 -- 成功；-1 -- 失败
*/
static ssize_t GPIOx_xio_write(struct file *pfile ,const char __user *buf ,size_t cnt ,loff_t *offt)
{
    int retvalue = 0;
    u8 wdata;
    u8 sta;
    /* 获取设备的私有数据结构 */
    /* 获取_gpioled数据结构，在这里可以通过dev访问设备的GPIO控制或LED状态等 */
    struct gpioled_dev *dev = pfile->private_data;
    /*  接收APP发送过来的数据 */
    retvalue = copy_from_user(&wdata ,buf ,cnt);
    if(retvalue < 0)
    {
        pr_notice("write kerneldata failed\r\n");
        return -1;
    }
    /* 获取状态值 */
    sta = wdata;
    if(sta == 0x01){
        gpio_set_value(dev->num ,1);
    }
    else{
        gpio_set_value(dev->num ,0);
    }
    return 0;
}
/*
* @function_name : chrdevbase_release
* @description   : 关闭/释放设备
* @param  pinode : 传递给驱动的 inode(节点，在文件创建时创建，包含了文件的元数据)
* @param  pfilp  : 设备文件，表示打开的文件描述符
* @return        : 0
*/
static int GPIOx_xio_release(struct inode *pinode,struct file *pfilp)
{
    return 0;
}
/* 设备操作函数:初始化GPIOx_xio_file*/
static struct file_operations GPIOI_PI3_led_file =
{
    .owner = THIS_MODULE,/* 绑定当前驱动模块 */
    .open = GPIOx_xio_open,
    .read = GPIOx_xio_read,
    .write = GPIOx_xio_write,
    .release = GPIOx_xio_release
};
/*
* @function_name : GPIOx_xio_init
* @description   : 驱动入口
* @return        : 0 - 成功/ -1 - 失败
*/
static int __init _gpioled_init(void)
{
    const char* str;            /* state属性值 */
/* 设置LED所使用的GPIO */
/* 1.获取设备节点：led-red */
    _gpioled.nd = of_find_node_by_name(NULL ,"led-red");
    if(_gpioled.nd == NULL){
        pr_err("node %s dose not exits\r\n" ,"led-red");
        return -EINVAL;
    }
/* 2.读取status属性 */
    if(of_property_read_string(_gpioled.nd ,"status" ,&str) < 0){
        pr_err("property %s read failed\r\n" ,"status");
        return -EINVAL;
    }
    if(strcmp(str ,"okay") != 0){
        pr_err("strcmp str and okay error");
        return -EINVAL;
    }
/* 3.获取设备树中的gpio属性，得到LED所使用的LED编号 */
    _gpioled.num = of_get_named_gpio(_gpioled.nd ,"gpios" ,0);
    if(_gpioled.num < 0){
        pr_err("can't get gpios\r\n");
        return -EINVAL;
    }
    pr_notice("led-gpio num = %d\r\n", _gpioled.num);
/* 4.向gpio子系统申请使用GPIO */
    if(gpio_request(_gpioled.num ,"pi3led")){
        pr_err("pi3led request failed\r\n");
        return -EINVAL;
    }
/* 5.设置PI3为输出，并且输出高电平，默认关闭LED灯 */
    if(gpio_direction_output(_gpioled.num ,1) < 0){
        pr_err("can't set gpio!\r\n");
    }
/*4.注册字符设备驱动*--------------------------------------------------------------*/
/*4.1 创建字符设备 */
    /* 已经定义设备号 */
    if(_gpioled.major)
    {
        /* 将主设备号和次设备号组合成一个dev_t类型的设备号 */
        _gpioled.devid = MKDEV(_gpioled.major ,0);
        /* 注册固定设备号 */
        if(register_chrdev_region(_gpioled.devid ,GPIOLED_CNT ,GPIOLED_NAME) < 0)
        {
            /* 注册失败 */
            pr_err("%s regsiter failed\r\n" ,GPIOLED_NAME);
            goto free_gpio;           
        }
    }
    /* 没有定义设备号 */
    else
    {
        /* 自动分配设备号 */
        if(alloc_chrdev_region(&_gpioled.devid ,0 ,GPIOLED_CNT ,GPIOLED_NAME) < 0)
        {
            /* 注册失败 */
            pr_err("%s regsiter failed\r\n" ,GPIOLED_NAME);
            goto free_gpio;  
        }
        /* 获取主设备号 */
        _gpioled.major = MAJOR(_gpioled.devid); /* 此代码可以省略 */
        /* 获取次设备号 */
        _gpioled.minor = MINOR(_gpioled.devid); /* 此代码可以省略 */
        pr_notice("%s: major = %d , minor = %d\r\n",GPIOLED_NAME ,_gpioled.major ,_gpioled.minor);
    }
    pr_notice("%s regsiter devid finished\r\n" ,GPIOLED_NAME);


/*4.2 创建字符设备小管家cdev */
    /* 将_gpioled._cdev字符设备的 owner 成员指向当前驱动模块 */
    _gpioled._cdev.owner = THIS_MODULE;
    /* 初始化cdev */
    cdev_init(&_gpioled._cdev ,&GPIOI_PI3_led_file);
    /* 向linux系统添加字符设备 */
    if(cdev_add(&_gpioled._cdev ,_gpioled.devid ,GPIOLED_CNT) < 0)
    {
        pr_err("%s cdev failed to add to linux\r\n",GPIOLED_NAME);
        goto del_devid;
    }
    pr_notice("%s cdev finished to add to linux\r\n",GPIOLED_NAME);

/*4.3 创建类:方便用户空间访问设备 */
    _gpioled._class = class_create(THIS_MODULE ,GPIOLED_NAME);
    if(IS_ERR(_gpioled._class))
    {
        pr_err("%s created class failed\r\n",GPIOLED_NAME);
        goto del_cdev;
    }
    pr_notice("%s created class finished\r\n",GPIOLED_NAME);

/*4.4 创建设备 */
    _gpioled._device = device_create(_gpioled._class ,NULL ,_gpioled.devid ,NULL ,GPIOLED_NAME);
    if(IS_ERR(_gpioled._device))
    {
        pr_err("%s created device failed\r\n",GPIOLED_NAME);
        goto del_class;
    }
    pr_notice("%s created device finished\r\n",GPIOLED_NAME);

    return 0;
/*------------------------------------------------------------------------------*/
/* 注销类 */
del_class:
    class_destroy(_gpioled._class);
/* 注销设备 */
del_cdev:
    cdev_del(&_gpioled._cdev);
/* 注销设备号 */
del_devid:
    unregister_chrdev_region(_gpioled.devid ,GPIOLED_CNT);
/* 释放gpio */
free_gpio:
    gpio_free(_gpioled.num);
    return -EINVAL;
}
/*
 * @description	: 驱动出口函数
 * @param 		: 无
 * @return 		: 无
 */
static void __exit _gpioled_exit(void)
{
    device_destroy(_gpioled._class ,_gpioled.devid);
    class_destroy(_gpioled._class);
    cdev_del(&_gpioled._cdev);
    unregister_chrdev_region(_gpioled.devid ,GPIOLED_CNT);
    gpio_free(_gpioled.num);
    pr_notice("%s logout finished\r\n",GPIOLED_NAME);
}

module_init(_gpioled_init);
module_exit(_gpioled_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("baotou");
MODULE_INFO(intree,"Y");
