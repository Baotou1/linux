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
/*----------------------------------------------------------------------------------*/
#define DEV_NAME          ("pf14-key-irq")
#define DEV_CNT           (0x01)
/* 按键状态值 */
#define KEY_PRESS         (0x00)
#define KEY_RELEASE       (0x01)
#define KEY_KEEP          (0x02) /* 按键状态中间态 */
/* 错误值 */
#define ERR_HW_ND         (0x01) 
#define ERR_HW_ND_PTY     (0x02) 
#define ERR_HW_IO_NUM     (0x03) 
#define ERR_HW_IO         (0x04) 
#define ERR_HW_IRQ        (0x05) 
#define ERR_DEV_ID        (0x0A) 
#define ERR_DEV_CDEV      (0x0B) 
#define ERR_DEV_CLS       (0x0C) 
#define ERR_DEV_DEV       (0x0D) 
/*--------------------------------------------------------------------*/
/**
 * struct struct_name - 按键驱动结构体
 * @member1: 
 * @member2: 
 */
typedef struct
{
    struct device_node* nd; /* 设备树节点 */
    int io;                 /* io编号 */
    struct cdev _cdv;       /* 设备字符驱动 */
    struct class* _cls;     /* 设备类 */
    struct device* _dev;    /* 设备 */
    dev_t id;               /* 设备id */
    int major;              /* 主设备号 */
    int minor;              /* 次设备号 */
    unsigned int irq;       /* 中断号 */
    atomic_t sta;           /* 按键状态 */
    u8 last_sta;            /* 默认状态 */
    struct timer_list tim;  /* 软件定时器 */
    wait_queue_head_t r_wait;	/* 读等待队列头 */
    struct fasync_struct *async_queue; /* 异步通知队列 */
}__dev;
__dev _devkey;

/*--------------------------------------------------------------------*/
/**
 * @function_name - 打开文件
 * @param1
 * @param2
 * @return
 */static int devkey_open(struct inode *nd, struct file *filp)
{
    filp->private_data = &_devkey;
    return 0;
}

/**
 * @function_name - 读取文件
 * @param1
 * @param2
 * @return
 */
static ssize_t devkey_read(struct file *filp, char __user *buf, size_t cnt, loff_t *lofft)
{
    __dev* udev = filp->private_data;
    int ret;
    if(cnt != sizeof(int)){
        pr_err("Invalid read size: expected %zu, got %zu\n", sizeof(int), cnt);
        return -ENAVAIL;
    }

#if 1
    /* 阻塞访问，判断条件 */
    ret = wait_event_interruptible(_devkey.r_wait ,KEY_KEEP != atomic_read(&_devkey.sta));
    if(ret)
        return ret;
#endif

    ret = copy_to_user(buf ,&udev->sta ,cnt);
    if(ret < 0){
        pr_err("err\r\n");
        return -ENAVAIL;
    }
    /* 状态重置 */
    atomic_set(&udev->sta ,KEY_KEEP);
    _devkey.last_sta = KEY_KEEP;
    return 1;
}

/**
 * @function_name - 异步通知文件
 * @param1
 * @param2
 * @return
 */
static int devkey_fasync(int fd, struct file *filp, int on)
{
    __dev *udev = filp->private_data;
    /* 初始化异步通知 */
    return fasync_helper(fd ,filp ,on ,&udev->async_queue);
}

/**
 * @function_name - 释放文件
 * @param1
 * @param2
 * @return
 */
static int devkey_release(struct inode *nd, struct file *filp)
{
    /* 传入-1是一种无影响的写法，表示“当前我们只是要删除，不关心fd” */
    return devkey_fasync(-1 ,filp ,0);
}

/**
 * struct struct_name - 按键操作结构体
 * @member1: 
 * @member2: 
 */
static struct file_operations fops= 
{
    .owner = THIS_MODULE,
    .open = devkey_open,
    .read = devkey_read,
    .release = devkey_release,
    .fasync = devkey_fasync
};
/*--------------------------------------------------------------------*/
/**
 * @function_name - 软件定时器回调函数
 * @param1
 * @param2
 * @return
 */
static void s_timfunc(struct timer_list *arg)
{
    static int keysta_f;
    int keysta_s;
    static u8 tim_flag = 0;
    /* 中断触发定时器 */
    if(tim_flag == 0){
        tim_flag = 1;
        keysta_f = gpio_get_value(_devkey.io);
        if((_devkey.last_sta == KEY_RELEASE && keysta_f == KEY_PRESS)
        || (_devkey.last_sta == KEY_PRESS && keysta_f == KEY_RELEASE)
        )
        {
            _devkey.last_sta = keysta_f;
            /* 设置定时器消抖 */
            mod_timer(&_devkey.tim ,jiffies + msecs_to_jiffies(20));
        }
    }

    /* 消抖触发定时器 */
    if(tim_flag == 1){
        tim_flag = 0;
        keysta_s = gpio_get_value(_devkey.io);
        /* 按键按下 */
        if(keysta_s == _devkey.last_sta && keysta_s == KEY_PRESS){
            atomic_set(&_devkey.sta ,KEY_PRESS);
            /* 唤醒队列头 */
            wake_up_interruptible(&_devkey.r_wait);
            if(_devkey.async_queue){
                /* 通过异步通知发送信号给应用空间 可读*/
                kill_fasync(&_devkey.async_queue , SIGIO ,POLL_IN);
            }
        }
        /* 按键松开 */
        else if(keysta_s == _devkey.last_sta && keysta_s == KEY_RELEASE){
            atomic_set(&_devkey.sta ,KEY_RELEASE);
            /* 唤醒队列头 */
            wake_up_interruptible(&_devkey.r_wait);
            if(_devkey.async_queue){
                /* 通过异步通知发送信号给应用空间 可读*/
                kill_fasync(&_devkey.async_queue , SIGIO ,POLL_IN);
            }
        }
        else{
            atomic_set(&_devkey.sta ,KEY_KEEP);
        } 
        _devkey.last_sta = keysta_s;   
    }
}

/**
 * @function_name - pf14硬件中断回调函数
 * @param1
 * @param2
 * @return
 */
static irqreturn_t key_pf14_interrupt(int irq, void *dev_id)
{
    /* 设置定时器 */
    mod_timer(&_devkey.tim, jiffies + msecs_to_jiffies(5));
    return IRQ_HANDLED;
}
/*--------------------------------------------------------------------*/
/**
 * @function_name - 解析设备树中相关硬件节点
 * @param1
 * @param2
 * @return
 */
static int _dt_init_key_pf14(void)
{
    int ret;
    unsigned long irq_flag;
    _devkey.nd = of_find_node_by_name(NULL ,"key1");
    if(!_devkey.nd)
        return -ERR_HW_ND;
   
    _devkey.io = of_get_named_gpio(_devkey.nd ,"gpios" ,0);
    if(_devkey.io < 0)
        return -ERR_HW_IO_NUM;

    ret = gpio_request(_devkey.io ,DEV_NAME);
    if(ret != 0)
        return -ERR_HW_IO;
        
    ret = gpio_direction_input(_devkey.io);
    if(ret < 0)
        return -ERR_HW_IO;
    
    /* 获取中断号 */
    _devkey.irq = irq_of_parse_and_map(_devkey.nd ,0);
    pr_notice(" _devkey.irq == %d \r\n",_devkey.irq);
    if(!_devkey.irq)
        return -ERR_HW_IO;

    /* 申请中断 */
    irq_flag = irq_get_trigger_type(_devkey.irq);
    if(irq_flag != IRQ_TYPE_EDGE_BOTH)
        irq_flag = IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING;
    ret = request_irq(_devkey.irq ,key_pf14_interrupt ,irq_flag ,"KEY1_PF14_IRQ" ,NULL);
    if(ret != 0)
        return -ERR_HW_IRQ;
    return 0;
}

/**
 * @function_name - 向内核注册驱动
 * @param1
 * @param2
 * @return
 */
static int _abs_k_key(void)
{
    int ret;
    ret = alloc_chrdev_region(&_devkey.id ,0 ,DEV_CNT ,DEV_NAME);
    if(ret < 0)
        return -ERR_DEV_ID;

    _devkey._cdv.owner = THIS_MODULE;
    cdev_init(&_devkey._cdv ,&fops);
    ret = cdev_add(&_devkey._cdv ,_devkey.id ,DEV_CNT);
    if(ret < 0)
        return -ERR_DEV_CDEV;

    _devkey._cls = class_create(THIS_MODULE ,DEV_NAME);
    if(IS_ERR(_devkey._cls))
        return -ERR_DEV_CLS;

    _devkey._dev = device_create(_devkey._cls ,NULL ,_devkey.id ,NULL ,DEV_NAME);    
    if(IS_ERR(_devkey._dev))
        return -ERR_DEV_DEV;

    return 0;
}
/*--------------------------------------------------------------------*/
/**
 * @function_name - 驱动入口
 * @param1
 * @param2
 * @return
 */
static int __init _key_init(void)
{
    int ret;
    _devkey.last_sta = KEY_RELEASE;
    /* 初始化按键状态 */
    atomic_set(&_devkey.sta ,KEY_KEEP);
    /* 等待队列头初始化 */
    init_waitqueue_head(&_devkey.r_wait);
    /* 初始化timer，设置定时器处理函数,还未设置周期，所有不会激活定时器 */
    timer_setup(&_devkey.tim ,s_timfunc ,0);
    ret = _dt_init_key_pf14();
    if(ret == -ERR_HW_ND && ret == -ERR_HW_IO_NUM)
        goto err_hd;
    else if(ret == -ERR_HW_IO)
        goto err_hd_io;
    else if(ret == -ERR_HW_IRQ)
        goto err_hd_irq;
    pr_notice("%s hd init ok\r\n" , DEV_NAME);
  
#if 1
    ret = _abs_k_key();
    if(ret == -ERR_DEV_ID)
        goto err_dev_id;
    else if(ret == -ERR_DEV_CDEV)
        goto err_dev_cdev;
    else if(ret == -ERR_DEV_CLS)
        goto err_dev_cls;
    else if(ret == -ERR_DEV_DEV)
        goto err_dev_dev;
    pr_notice("%s dev abs k ok\r\n" ,DEV_NAME);
#endif
	return 0;
err_dev_dev:
    device_destroy(_devkey._cls ,_devkey.id);
err_dev_cls:
    class_destroy(_devkey._cls);
err_dev_cdev:
    cdev_del(&_devkey._cdv);
err_dev_id:
    unregister_chrdev_region(_devkey.id ,DEV_CNT);
err_hd_irq:
    free_irq(_devkey.irq ,_devkey.nd);
err_hd_io:
    gpio_free(_devkey.io);
err_hd:
    pr_err("%s hd init error\r\n" , DEV_NAME);
    return -ENAVAIL;
}

/**
 * @function_name - 驱动出口
 * @param1
 * @param2
 * @return
 */
static void __exit _key_exit(void)
{
    device_destroy(_devkey._cls ,_devkey.id);
    class_destroy(_devkey._cls);
    cdev_del(&_devkey._cdv);
    unregister_chrdev_region(_devkey.id ,DEV_CNT);
    free_irq(_devkey.irq ,NULL);
    gpio_free(_devkey.io);
    pr_notice("%s Unload Finished\r\n" ,DEV_NAME);
}

module_init(_key_init);
module_exit(_key_exit);
/*--------------------------------------------------------------------*/
MODULE_LICENSE("GPL");
MODULE_AUTHOR("baotot");
MODULE_INFO(intree,"Y");