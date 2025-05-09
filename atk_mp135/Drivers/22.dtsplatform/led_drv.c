#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/gpio.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/semaphore.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/kdev_t.h>
#include <linux/of_irq.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <asm/mach/map.h>
#include <asm/uaccess.h>
#include <linux/of_gpio.h>
#include <asm/io.h>
#include <linux/wait.h>
#include <linux/poll.h>
#include <linux/fs.h>
#include <linux/platform_device.h>
#include <linux/ioport.h>
/*----------------------------------------------------------------------------------*/
#define LEDDEV_NAME "pi3-led"
#define LEDDEV_LEN  (0x01)
#define ECHR_ID     (0xA1)
#define ECHR_CDV    (0xA2)
#define ECHR_CLS    (0xA3)
#define ECHR_DEV    (0xA4)
/*----------------------------------------------------------------------------------*/
static int pi3led_drv_open(struct inode *nd, struct file *filp);
static ssize_t pi3led_drv_write(struct file *filp, const char __user *buf, size_t size, loff_t *loft);
static ssize_t pi3led_drv_read(struct file *filp, char __user *buf, size_t size, loff_t *loft);
static int pi3led_drv_release(struct inode *nd, struct file *filp);
static int plt_pi3led_probe(struct platform_device *dev);
static int plt_pi3led_remove(struct platform_device *dev);
/*----------------------------------------------------------------------------------*/
/**
 * struct struct_name - 字符驱动相关参数结构体
 * @member1: 
 * @member2: 
 */
typedef struct{
    dev_t id;
    struct cdev _cdv;
    struct class *_cls;
    struct device *_dev;
}_chrdevbase;

/**
 * struct struct_name - 设备树相关参数结构体
 * @member1: 
 * @member2: 
 */
typedef struct{
    int num;
    struct device_node *np;
    unsigned int irq;
}_dtsbase;

/**
 * struct struct_name - led驱动结构体
 * @member1: 
 * @member2: 
 */
struct __led_drv{
    _chrdevbase _chr;
    _dtsbase _dts;
    atomic_t sta;
};
static struct __led_drv _led_drv;

/**
 * struct struct_name - led设备树匹配表结构体
 * @member1:
 * @member2: 
 */
static const struct of_device_id pi3led_of_match[] = {
    {
        .compatible = "pi3,gpio-led"
    },
    {
        /* 保留 */
    }
};
MODULE_DEVICE_TABLE(of ,pi3led_of_match);

/**
 * struct struct_name - led的platform驱动结构体
 * @member1: 
 * @member2: 
 */
static struct platform_driver pi3led_pltdrv = {
    .driver = {
        .name = "stm32mp135-pi3-led",
        .of_match_table = pi3led_of_match
    },
    .probe = plt_pi3led_probe,
    .remove = plt_pi3led_remove
};

/**
 * struct struct_name - led文件操作结构体
 * @member1: 
 * @member2: 
 */
static struct file_operations fops = {
    .open = pi3led_drv_open,
    .write = pi3led_drv_write,
    .read = pi3led_drv_read,
    .release= pi3led_drv_release
};

/*----------------------------------------------------------------------------------*/
/**
 * @function_name - 解析设备树中相关硬件节点
 * @param1
 * @param2
 * @return
 */
static int _dts_hd_init_pi3(void){
    int ret;
    u8 data;
    /* 获取gpio编号 */
    _led_drv._dts.num = of_get_named_gpio(_led_drv._dts.np ,"led-gpio" ,0);
    if(!gpio_is_valid(_led_drv._dts.num)){
        pr_err("%s get gpio-num error\r\n" ,LEDDEV_NAME);
        return -EIO;
    }
    /* 申请使用gpio */
    ret = gpio_request(_led_drv._dts.num ,LEDDEV_NAME);
    if(ret < 0){
        pr_err("%s gpio-request error\r\n" ,LEDDEV_NAME);
        return ret;
    }
    /* 设置gpio电平 */
    data = atomic_read(&_led_drv.sta); 
    gpio_direction_output(_led_drv._dts.num ,data);
    return 0;
}

/**
 * @function_name - 注册驱动到内核
 * @param1
 * @param2
 * @return
 */
static int _register_drv_to_kernel(void){
    int ret;
    /* 获取设备id */
    ret = alloc_chrdev_region(&_led_drv._chr.id ,0 ,LEDDEV_LEN ,LEDDEV_NAME);
    if(ret < 0){
        pr_err("%s chrdev-id-error\r\n" ,LEDDEV_NAME);
        return -ECHR_ID;
    }
    /* 注册字符驱动 */
    _led_drv._chr._cdv.owner = THIS_MODULE;
    cdev_init(&_led_drv._chr._cdv ,&fops);
    ret = cdev_add(&_led_drv._chr._cdv ,_led_drv._chr.id ,LEDDEV_LEN);
    if(ret < 0){
        pr_err("%s chrdev-cdev-error\r\n" ,LEDDEV_NAME);
        return -ECHR_CDV;
    }
    /* 注册类 */
    _led_drv._chr._cls = class_create(_led_drv._chr._cdv.owner ,LEDDEV_NAME);
    if(IS_ERR(_led_drv._chr._cls)){
        pr_err("%s chrdev-cls-error\r\n" ,LEDDEV_NAME);
        return -ECHR_CLS;
    }
    /* 注册设备 */
    _led_drv._chr._dev = device_create(_led_drv._chr._cls ,NULL ,_led_drv._chr.id ,NULL ,LEDDEV_NAME);
    if(IS_ERR(_led_drv._chr._dev)){
        pr_err("%s chrdev-dev-error\r\n" ,LEDDEV_NAME);
        return -ECHR_DEV;
    }
    return 0;
}

/**
 * @function_name - led contrl
 * @param1
 * @param2
 * @return
 */
static void led_contrl(struct __led_drv *udrv){
    int sta;
    sta = atomic_read(&udrv->sta);
    if(sta == 0x01){
        gpio_set_value(udrv->_dts.num ,sta);
    }
    else{
        gpio_set_value(udrv->_dts.num ,sta);
    }
}

/*----------------------------------------------------------------------------------*/
/**
 * @function_name - 打开文件
 * @param1
 * @param2
 * @return
 */
static int pi3led_drv_open(struct inode *np, struct file *filp){
    filp->private_data = &_led_drv;
    return 0;
}

/**
 * @function_name - 写入文件
 * @param1
 * @param2
 * @return
 */
static ssize_t pi3led_drv_write(struct file *filp, const char __user *buf, size_t cnt, loff_t *loft){
    int ret = 0;
    u8 wdata;
    struct __led_drv *udrv = filp->private_data;
    if(cnt != 1){
        pr_err("write cnt -EINVAL\r\n");
        return -EINVAL;
    }
    /* 向内核空间写入数据 */
    ret = copy_from_user(&wdata ,buf ,cnt);
    if(ret < 0)
    {
        pr_err("write kerneldata failed\r\n");
        return -1;
    }

    atomic_set(&udrv->sta ,wdata);

    led_contrl(udrv);
    return 0;
}

/**
 * @function_name - 读取文件
 * @param1
 * @param2
 * @return
 */
static ssize_t pi3led_drv_read(struct file *filp, char __user *buf, size_t cnt, loff_t *loft){
    int ret = 0;
    int rdata;
    struct __led_drv *udrv = filp->private_data;
    if(cnt != 1){
        pr_err("read cnt -EINVAL\r\n");
        return -EINVAL;
    }

    rdata = gpio_get_value(udrv->_dts.num);
    atomic_set(&udrv->sta ,rdata);

    ret = copy_to_user(buf ,&udrv->sta ,1);
    if(ret < 0){
        pr_err("read kerneldata failed\r\n");
        return -1;
    }
    return 0;
}

/**
 * @function_name - 关闭文件
 * @param1
 * @param2
 * @return
 */
static int pi3led_drv_release(struct inode *np, struct file *filp){
    return 0;
}

/**
 * @function_name - 注册platform驱动回调函数
 * @param1
 * @param2
 * @return
 */
static int plt_pi3led_probe(struct platform_device *pdev){
    int ret;
    _led_drv._dts.np = pdev->dev.of_node;

    if(_led_drv._dts.np == NULL){
        pr_err("%s get gpio-drv-node error\r\n" ,LEDDEV_NAME);
        return -ENXIO;
    }

    _led_drv.sta = (atomic_t)ATOMIC_INIT(0);
    /* 设置为高电平*/
    atomic_set(&_led_drv.sta ,0x01);

    ret = _dts_hd_init_pi3();
    if(ret < 0){
        return ret;
    }
    pr_notice("%s gpio-init success\r\n" ,LEDDEV_NAME);

    ret = _register_drv_to_kernel();
    if(ret == -ECHR_ID)
        goto echr_id;
    else if(ret == -ECHR_CDV)
        goto echr_cdv;
    else if(ret == -ECHR_CLS)
        goto echr_cls;
    else if(ret == -ECHR_DEV)
        goto echr_dev;

    pr_notice("%s cdev-register-kernel-init success\r\n" ,LEDDEV_NAME);
    return 0;
echr_dev:
    device_destroy(_led_drv._chr._cls ,_led_drv._chr.id);
echr_cls:
    class_destroy(_led_drv._chr._cls);
echr_cdv:
    cdev_del(&_led_drv._chr._cdv);
echr_id:
    unregister_chrdev_region(_led_drv._chr.id ,LEDDEV_LEN);
    gpio_free(_led_drv._dts.num);
    return -EIO;
}

/**
 * @function_name - 移除platform驱动回调函数
 * @param1
 * @param2
 * @return
 */
static int plt_pi3led_remove(struct platform_device *pdev){
    device_destroy(_led_drv._chr._cls ,_led_drv._chr.id);
    class_destroy(_led_drv._chr._cls);
    cdev_del(&_led_drv._chr._cdv);
    unregister_chrdev_region(_led_drv._chr.id ,LEDDEV_LEN);
    gpio_free(_led_drv._dts.num);
    pr_notice("%s unload finished\r\n" ,LEDDEV_NAME);
    return 0;
};

/*----------------------------------------------------------------------------------*/
/**
 * @function_name - 驱动入口
 * @param1
 * @param2
 * @return
 */
static int __init pi3led_drv_init(void){
    return platform_driver_register(&pi3led_pltdrv);
}

/**
 * @function_name - 驱动出口
 * @param1
 * @param2
 * @return
 */
static void __exit pl3led_drv_exit(void){
    platform_driver_unregister(&pi3led_pltdrv);
}

/*----------------------------------------------------------------------------------*/
module_init(pi3led_drv_init);
module_exit(pl3led_drv_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("baotou");
MODULE_INFO(intree, "Y");