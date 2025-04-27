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
    u8 irq_flag;            /* 中断flag */
    spinlock_t slock;       /* 自旋锁 */
    int sta;                /* 当前按键状态 */
    int last_sta;           /* 上一次按键状态 */
    struct timer_list tim;  /* 软件定时器 */
    int tim_prd;            /* 定时器周期 */
    u8 tim_flag;            /* 定时器标志 */
}__dev;
__dev _devkey;
/*--------------------------------------------------------------------*/
static int devkey_open(struct inode *nd, struct file *f)
{
    f->private_data = &_devkey;
    return 0;
}
static ssize_t devkey_read(struct file *f, char __user *buf, size_t cnt, loff_t *lofft)
{
    __dev* udev = f->private_data;
    int new_sta;
    unsigned long flags;
    int ret;
    if(cnt != sizeof(int)){
        pr_err("Invalid read size: expected %zu, got %zu\n", sizeof(int), cnt);
        return -ENAVAIL;
    }
    spin_lock_irqsave(&udev->slock ,flags);
    if(udev->irq_flag == 1)
    {
        udev->irq_flag = 0;
        new_sta = gpio_get_value(udev->io);
        if( (new_sta == KEY_PRESS && udev->last_sta == KEY_RELEASE)
            || (new_sta == KEY_RELEASE && udev->last_sta == KEY_PRESS))
        {
            udev->last_sta = new_sta;
            /* 设置定时器 */
            mod_timer(&udev->tim ,jiffies + msecs_to_jiffies(udev->tim_prd));
        }
    }
    else
    {
        udev->sta = KEY_KEEP;
    }

    if(udev->tim_flag == 1)
    {
        udev->tim_flag = 0;
        new_sta = gpio_get_value(udev->io);
        if(new_sta == udev->last_sta)
        {
            udev->sta = new_sta;
        }
        else{
            udev->sta = KEY_KEEP;
        }
        udev->last_sta = new_sta;
    }
    spin_unlock_irqrestore(&udev->slock,flags); 
    ret = copy_to_user(buf ,&udev->sta ,cnt);
    if(ret < 0){
        pr_err("err\r\n");
        return -ENAVAIL;
    }
    return 1;
}
static int devkey_release(struct inode *nd, struct file *f)
{
    return 0;
}

static struct file_operations fops= 
{
    .owner = THIS_MODULE,
    .open = devkey_open,
    .read = devkey_read,
    .release = devkey_release
};
/*--------------------------------------------------------------------*/
/* 中断回调函数 */
static irqreturn_t key_pf14_interrupt(int irq, void *dev_id)
{
    _devkey.irq_flag = 1;
    return IRQ_HANDLED;
}
/* 定时器回调函数 */
static void s_timfunc(struct timer_list *arg)
{
    _devkey.tim_flag = 1;
}
/*--------------------------------------------------------------------*/
/* 自旋锁初始化 */
static void _s_lock_init(void)
{
    /* 设置按键初始状态为中间态 */
    _devkey.last_sta = KEY_RELEASE;
    spin_lock_init(&_devkey.slock);
}
/* 定时器初始化 */
static void _s_tim_init(void)
{
    timer_setup(&_devkey.tim ,s_timfunc ,0);
    /* 定时器周期30ms */
    _devkey.tim_prd = 30;
}
/* 设备树中按键初始化 */
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

/* 向内核注册驱动 */
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
static int __init _key_init(void)
{
    int ret;
    _s_lock_init();
    _s_tim_init();
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