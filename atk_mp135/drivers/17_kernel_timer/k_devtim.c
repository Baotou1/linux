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
#include <linux/of_gpio.h>
#include <linux/of_address.h>
#include <linux/kdev_t.h>
#include <linux/semaphore.h>
#include <linux/timer.h>
#include <asm/mach/map.h>
#include <asm/uaccess.h>
#include <asm/io.h>
/*----------------------------------------------------------------------------------*/
#define DEV_NAME          ("led-pi3-tim")
#define DEV_CNT           (0x01)
/*命令值*/
/*_IO(0xEF, 0x01)=0xEF01=61186*/
#define CMD_OPEN		  (_IO(0XEF, 0x01))	/* 打开定时器 */
#define CMD_SETPERIOD	  (_IO(0XEF, 0x02))	/* 设置定时器周期命令 */
#define CMD_CLOSE		  (_IO(0XEF, 0x03))	/* 关闭定时器 */
/*错误值*/
#define ERR_HW_ND         (0x01) 
#define ERR_HW_ND_PTY     (0x02) 
#define ERR_HW_IO_NUM     (0x03) 
#define ERR_IO            (0x04) 
#define ERR_DEV_ID        (0x0A) 
#define ERR_DEV_CDEV      (0x0B) 
#define ERR_DEV_CLS       (0x0C) 
#define ERR_DEV_DEV       (0x0D) 
/*----------------------------------------------------------------------------------*/
typedef struct
{
    /*设备树节点*/
    struct device_node *_nd;
    /*节点对应引脚编号*/
    int num;
    /*内核对应设备*/
    struct device *_dev;
    /*内核对应类*/
    struct class *_cls;
    /*内核对应驱动*/
    struct cdev _cdv;
    /*主设备号*/
    int major;
    /*次设备号*/
    int minor;
    /*设备ID*/
    dev_t id;
    /* 备树初始化硬件 */
    int (*_dt_init_hw)(void);
    /* 抽象到内核 */
    int (*_abs_k)(void);
    /*内核定时器*/
    struct timer_list _tim;
    /*内核定时器周期时间*/
    int tim_prd;
    /*内核定时器回调函数*/
    void (*_timfunc)(struct timer_list *arg);
    /*自旋锁 保护结构体中的变量tim_prd*/
    spinlock_t _slock;
}__udev;
__udev dev_led;

/*----------------------------------------------------------------------------------*/
/*
 * @function_name   : _open_led_tim
 * @description     : 打开设备
 * @param - nd  	: 驱动文件节点(/dev/dev)
 * @param - filp 	: 要打开的设备文件(文件描述符)
 * @return 			: 0 成功;其他 失败
*/
static int _open_led_tim(struct inode *nd ,struct file *fp )
{
    fp->private_data = &dev_led;
    /*设置周期时间1s*/
    dev_led.tim_prd = 1000;
    return 0;
}
/*
 * @function_name   : _unlocked_ioctl_led_tim
 * @description     : 控制定时器
 * @param - filp 	: 要打开的设备文件(文件描述符)
 * @param - cmd 	: 应用程序发送过来的命令
 * @param - arg 	: 参数
 * @return 			: 0 成功;其他 失败
*/
static long _unlocked_ioctl_led_tim(struct file *fp, unsigned int cmd, unsigned long arg)
{
    unsigned long flags;
    int timprd;
    __udev *dev = fp->private_data;
    if(cmd == CMD_OPEN){ /*打开定时器*/
        spin_lock_irqsave(&dev->_slock ,flags);
        timprd = dev->tim_prd;
        spin_unlock_irqrestore(&dev->_slock ,flags);
        mod_timer(&dev->_tim ,jiffies + msecs_to_jiffies(timprd));
    }
    else if(cmd == CMD_SETPERIOD){/*设置定时器周期*/
        spin_lock_irqsave(&dev->_slock ,flags);
        dev->tim_prd = arg;
        spin_unlock_irqrestore(&dev->_slock ,flags);
        mod_timer(&dev->_tim ,jiffies + msecs_to_jiffies(dev->tim_prd));
    }
    else if(cmd == CMD_CLOSE){/*关闭定时器*/
        del_timer_sync(&dev->_tim);
    }
    return 0;
}
/*
 * @function_name   : _release_led_tim
 * @description     : 关闭/释放设备
 * @param - nd  	: 驱动文件节点(/dev/dev)
 * @param - filp 	: 要关闭的设备文件(文件描述符)
 * @return 			: 0 成功;其他 失败
*/
int _release_led_tim(struct inode *nd, struct file *fp)
{
    __udev *dev = fp->private_data;
    /* APP结束的时候关闭LED */
    gpio_set_value(dev->num ,1);
    del_timer_sync(&dev->_tim);
    return 0;
}

struct file_operations _fops_led = {
    .open = _open_led_tim,
    .unlocked_ioctl = _unlocked_ioctl_led_tim,
    .release = _release_led_tim
};
/*----------------------------------------------------------------------------------*/
/*
* @function_name : _led_timfunc
* @description   : 关于led定时器的回调函数
* @return        : 无
*/
static void _led_timfunc(struct timer_list *arg)
{
    /* from_timer是Linux内核中一个宏，常用于定时器回调函数中获取包含该定时器的结构体指针。
     * 第一个参数表示结构体
     * 第二个参数表示第一个参数里的一个成员
     * 第三个参数表示第二个参数的类型
     * 通过timer_list指针 t，反推出它所在的结构体dev指针*/
    __udev *dev = from_timer(dev ,arg ,_tim);
    int timprd;
    unsigned long flags;
    static u8 sta = 1;

    if(!dev){
        return;
    }
    /*led状态取反*/
    sta = !sta;
    /*设置led*/
    gpio_set_value(dev->num ,sta);

    /*加锁*/
    spin_lock_irqsave(&dev->_slock ,flags);
    /*设置定时器周期性时间*/
    timprd = dev->tim_prd;
    /*取锁*/
    spin_unlock_irqrestore(&dev->_slock ,flags);
    /*重启定时器*/
    mod_timer(&dev->_tim ,jiffies + msecs_to_jiffies(timprd));
}
/*----------------------------------------------------------------------------------*/
/*
* @function_name : _dt_init_led
* @description   : 通过设备树初始化led
* @return        : 0 
*/
static int _dt_init_led(void)
{
    const char *hw_nd_pty;

    dev_led._nd = of_find_node_by_name(NULL ,"led-red");
    if(IS_ERR(dev_led._nd)){
        pr_err("hw_nnd not find\r\n");
        return -ERR_HW_ND;
    }
    pr_notice("hw_nd find\r\n");

    of_property_read_string(dev_led._nd ,"status" ,&hw_nd_pty);
    if(strcmp(hw_nd_pty ,"okay") != 0){
        pr_err("hw_nd_pty not mismatch\r\n");
        return -ERR_HW_ND_PTY;
    }
    pr_notice("hw_nd_pty match\r\n");

    dev_led.num = of_get_named_gpio(dev_led._nd ,"gpios" ,0);
    if(!dev_led.num){
        pr_err("hw_io_num err\r\n");
        return -ERR_HW_IO_NUM;
    }
    pr_notice("hw_io_num = %d\r\n",dev_led.num);

    if(gpio_request(dev_led.num ,"io-led-pi3") != 0){
        pr_err("io request err\r\n");
        return -ERR_IO;
    }
    pr_notice("io request ok\r\n");

    if(gpio_direction_output(dev_led.num ,1) < 0){
        pr_err("io err\r\n");
        return -ERR_IO;
    }
    pr_notice("hw init ok\r\n");
    return 1;
}
/*
* @function_name : _abs_k_led
* @description   : 将led硬件设备抽象到内核
* @return        : 0 
*/
static int _abs_k_led(void)
{
    int ret = 0;
    /* 获取设备id号 */
    ret = alloc_chrdev_region(&dev_led.id ,0 ,DEV_CNT ,DEV_NAME);
    if(ret < 0){
        pr_err("dev id error\r\n");
        return -ERR_DEV_ID;
    }
    dev_led.major = MAJOR(dev_led.id);
    dev_led.minor = MINOR(dev_led.id);
    pr_notice("led: major = %d minor = %d\r\n" ,dev_led.major ,dev_led.minor);

    /* 注册字符驱动到内核 */
    dev_led._cdv.owner = THIS_MODULE;
    cdev_init(&dev_led._cdv ,&_fops_led);
    ret = cdev_add(&dev_led._cdv ,dev_led.id ,DEV_CNT);
    if(ret < 0){
        pr_err("cdev add error\r\n");
        return -ERR_DEV_CDEV;
    }
    pr_notice("cdev add ok\r\n");

    /* 内核分配类 */
    dev_led._cls = class_create(THIS_MODULE ,DEV_NAME);
    if(IS_ERR(dev_led._cls)){
        pr_err("class error\r\n");
        return -ERR_DEV_CLS;
    }
    pr_notice("class ok\r\n");

    /* 抽象出用户空间设备 */
    dev_led._dev = device_create(dev_led._cls ,NULL ,dev_led.id ,NULL ,DEV_NAME);
    if(IS_ERR(dev_led._dev)){
        pr_err("device error\r\n");
        return -ERR_DEV_DEV;
    }
    pr_notice("device ok\r\n");
    return 1;
}
/*----------------------------------------------------------------------------------*/
/*
* @function_name : _dev_init
* @description   : 驱动入口函数
* @return        : 0 
*/
static int __init _dev_init(void)
{
    int ret;
    /* 初始化自旋锁 */
    spin_lock_init(&dev_led._slock);
    /* 内核定时器初始化 */
    dev_led._timfunc = _led_timfunc;
    timer_setup(&dev_led._tim ,dev_led._timfunc ,0);

    dev_led._dt_init_hw = _dt_init_led;
    dev_led._abs_k = _abs_k_led;

    ret = dev_led._dt_init_hw();
    if(ret == -ERR_HW_ND && ret == -ERR_HW_ND_PTY && ret == -ERR_HW_IO_NUM){
        goto err_hd;
    }
    else if (ret == -ERR_IO){
        goto err_hd_io;
    }

    ret = dev_led._abs_k();
    switch(ret)
    {
        case -ERR_DEV_ID:
            goto err_dev_id;
        case -ERR_DEV_CDEV:
            goto err_dev_cdev;
        case -ERR_DEV_CLS:
            goto err_dev_cls;
        case -ERR_DEV_DEV:
            goto err_dev_dev;
    }
    return 0;
err_dev_dev:
    device_destroy(dev_led._cls ,dev_led.id);
err_dev_cls:
    class_destroy(dev_led._cls);
err_dev_cdev:
    cdev_del(&dev_led._cdv);
err_dev_id:
    unregister_chrdev_region(dev_led.id ,DEV_CNT);
err_hd_io:
    gpio_free(dev_led.num);
err_hd:
    return -EINVAL;
}
/*
* @function_name : _dev_exit
* @description   : 驱动出口函数
* @return        : 0 
*/
static void __exit _dev_exit(void)
{
    device_destroy(dev_led._cls ,dev_led.id);
    class_destroy(dev_led._cls);
    cdev_del(&dev_led._cdv);
    unregister_chrdev_region(dev_led.id ,DEV_CNT);
    gpio_free(dev_led.num);
    pr_notice("%s Unload Finished\r\n",DEV_NAME);
}

module_init(_dev_init);
module_exit(_dev_exit);
/*----------------------------------------------------------------------------------*/
/* 模块许可证 */
MODULE_LICENSE("GPL");
/* 模块作者 */
MODULE_AUTHOR("baotot");
/*  */
MODULE_INFO(intree,"Y");
