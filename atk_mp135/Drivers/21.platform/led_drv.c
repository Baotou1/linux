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
/* LED */
#define LEDDEV_NAME                ("platled_pi3")
#define LEDDEV_CNT                 (0x01)
/* 错误值 */
#define ERR_DEV_ID              (0x0A) 
#define ERR_DEV_CDEV            (0x0B) 
#define ERR_DEV_CLS             (0x0C) 
#define ERR_DEV_DEV             (0x0D) 
/* 寄存器物理地址 */
#define PERIPH_BASE_ADDR        (0x40000000)                            /*外设寄存器基地址*/
#define MPU_AHPB4_BASE_ADDR     (PERIPH_BASE_ADDR + 0x10000000)         /*MPU访问AHB4（总连接的外设（如GPIO、RCC））外设的基地址*/
#define RCC_BASE_ADDR           (MPU_AHPB4_BASE_ADDR + 0x00)            /*RCC时钟控制器基地址，用于管理系统时钟*/
#define RCC_MP_S_AHB4ENSETR     (RCC_BASE_ADDR + 0XA28)                 /*AHB4 外设时钟使能寄存器:控制GPIO端口*/
#define GPIOI_BASE              (MPU_AHPB4_BASE_ADDR + 0xA000)         /*GPIOI 外设基地址*/
#define GPIOI_OFFEST_ADDR       (0x04)                                  /*GPIOI 寄存器的地址偏移量*/
/* 端口模式寄存器：选择GPIO模式（输入、输出、复用、模拟）*/
#define GPIOI_MODER             (GPIOI_BASE + (GPIOI_OFFEST_ADDR * 0))  /*GPIOI_MODER地址，偏移量为0x00*/
/* 端口输出寄存器：选择GPIO输出模式（推挽、开漏） */
#define GPIOI_OTYPER            (GPIOI_BASE + (GPIOI_OFFEST_ADDR * 1))  /*GPIOI_OTYPER地址，偏移量为0x04*/
/* 端口输出速度寄存器 */
#define GPIOI_OSPEEDR           (GPIOI_BASE + (GPIOI_OFFEST_ADDR * 2))  /*GPIOI_OSPEEDR地址，偏移量为0x08*/
/* 端口上拉/下拉寄存器 */
#define GPIOI_PUPDR             (GPIOI_BASE + (GPIOI_OFFEST_ADDR * 3))  /*GPIOI_PUPDR寄存器地址，偏移量为0x0C*/
/* 端口输入数据寄存器: 读取 GPIO 引脚的当前电平状态 */
#define GPIOI_IDR               (GPIOI_BASE + (GPIOI_OFFEST_ADDR * 4))  /*GPIOI_IDR地址，偏移量为0x10*/
/* 端口输出数据寄存器: 设置 GPIO 引脚的输出电平（0 = 低电平，1 = 高电平） */
#define GPIOI_ODR               (GPIOI_BASE + (GPIOI_OFFEST_ADDR * 5))  /*GPIOI_ODR地址，偏移量为0x14*/
/* 端口位设置/复位寄存器: 一次性写入寄存器即可独立控制 GPIO 引脚的置位或复位*/
#define GPIOI_BSRR              (GPIOI_BASE + (GPIOI_OFFEST_ADDR * 6))  /*GPIOI_BSRR地址，偏移量为0x18*/
/* 映射后的寄存器虚拟地址指针 */
static void __iomem *PMPU_AHPB4_BASE;
static void __iomem *PGPIOI_MODER;
static void __iomem *PGPIOI_OTYPER;
static void __iomem *PGPIOI_OSPEEDR;
static void __iomem *PGPIOI_PUPDR;
static void __iomem *PGPIOI_IDR;
static void __iomem *PGPIOI_ODR;
static void __iomem *PGPIOI_BSRR;
/*----------------------------------------------------------------------------------*/
struct _led_dev{
    dev_t id;
    struct cdev cdv;
    struct class *cls;
    struct device *dev;
};
struct _led_dev pi3led_dev;

/*
* @function_name : io_control
* @description   : io口控制输出高低电平
* @param - sta   : 0x00 - 低电平
*                  0x01 - 高电平
* @return        : 无
*/
static void io_control(u8 sta)
{   
   u32 val;
   /* 输出高电平 */
   if(sta == 0x01)
   {
       /* BSRR寄存器的低15位BS[15：0]用于引脚设置为1 */
       val |= (1 << 3);
       /* 写入寄存器 */
       writel(val ,PGPIOI_BSRR);
   }
   /* 输出低电平 */
   else
   {
       /* BSRR寄存器的高15位BR[15：0]用于引脚复位为0 */
       val |= (1 << (3 + 0x10));
       writel(val ,PGPIOI_BSRR);
   }
}
static int pi3led_open(struct inode *nd, struct file *filp){
    return 0;
}
static ssize_t pi3led_write(struct file *filp, const char __user *buf, size_t cnt, loff_t *offt){
    int retvalue = 0;
    u8 wdata;
    u8 sta;
    /* 向内核空间写入数据 */
    retvalue = copy_from_user(&wdata ,buf ,cnt);
    printk("wdata = %d\r\n",wdata);
    if(retvalue < 0)
    {
        printk(KERN_NOTICE "write kerneldata failed\r\n");
        return -1;
    }
    /* 获取状态值 */
    sta = wdata;
    if(sta == 0x01){
        io_control(sta);
    }
    else{
        io_control(sta);
    }
    return 0;
}

static ssize_t pi3led_read(struct file *filp,char __user *buf, size_t cnt, loff_t *offt){
    int retvalue = 0;
    u8 io_sta;
    if(cnt < 1)
    {
        return -1;
    }
    /* 获取单个io口的值 */
    io_sta = (readl(PGPIOI_IDR) >> 3) & 0x01;
    if(cnt == 1)
    {
        /* 读取内核空间的数据 */
        retvalue = copy_to_user(buf ,&io_sta ,1);
        /* buf 是用户空间指针，不能直接解引用 *buf，否则会导致内核崩溃 */
        printk(KERN_INFO "io_sta == %d\r\n" ,io_sta);
        if(retvalue < 0){
            return -1;
        }
    }
    return 0;
}

struct file_operations fops = {
    .open = pi3led_open,
    .write = pi3led_write,
    .read = pi3led_read
};
/*----------------------------------------------------------------------------------*/
static void pi3led_unmap(void)
{
    iounmap(PMPU_AHPB4_BASE);
    iounmap(PGPIOI_MODER);
    iounmap(PGPIOI_OTYPER);
    iounmap(PGPIOI_OSPEEDR);
    iounmap(PGPIOI_PUPDR);
    iounmap(PGPIOI_IDR);
    iounmap(PGPIOI_ODR);
    iounmap(PGPIOI_BSRR);
}
/* 向内核注册驱动 */
static int _abs_k_key(void){
    int ret;
    /* 获取设备id */
    ret = alloc_chrdev_region(&pi3led_dev.id ,0 ,LEDDEV_CNT ,LEDDEV_NAME);
    if(ret < 0)
        return -ERR_DEV_ID;

    /* 注册字符驱动 */
    pi3led_dev.cdv.owner = THIS_MODULE;
    cdev_init(&pi3led_dev.cdv ,&fops);
    ret = cdev_add(&pi3led_dev.cdv ,pi3led_dev.id ,LEDDEV_CNT);
    if(ret < 0)
        return -ERR_DEV_CDEV;

    /* 创建块 */
    pi3led_dev.cls = class_create(THIS_MODULE ,LEDDEV_NAME);
    if(IS_ERR(pi3led_dev.cls))
        return -ERR_DEV_CLS;
    
    /* 创建设备 */
    pi3led_dev.dev = device_create(pi3led_dev.cls ,NULL ,pi3led_dev.id ,NULL ,LEDDEV_NAME);
    if(IS_ERR(pi3led_dev.dev))
        return -ERR_DEV_DEV;
    return 0;
}
/* 设备与驱动匹配成功时调用的回调函数 */
static int pi3led_probe(struct platform_device *dev){
    int i ,val ,ret;
    int resources_size[8];/* 保存每个寄存器长度 */
    struct resource *pi3led_resources[8];/* 保存关于Pi3led的寄存器地址 */
    
    pr_notice("pi3 led device and driver has matched successful\r\n");

    /* 获取 platform 设备资源的寄存器地址 */
    for(i = 0; i < 8; i++){
        /* 注意platform_get_resource中的第三个参数为i */
        pi3led_resources[i] = platform_get_resource(dev , IORESOURCE_MEM, i);
        if(!pi3led_resources[i]){
            dev_err(&dev->dev,"No MEM resource for always on\r\n");
            return -ENXIO;/* 表示“没有这样的设备或地址” */
        }
        resources_size[i] = resource_size(pi3led_resources[i]);
    }
    /* 映射寄存器地址 */
#if 1
    printk("resources_size = %d\r\n",resources_size[0]);
    for(i = 0; i < 8; i++){
        printk("pi3led_resources[%d] = %x\r\n",i ,pi3led_resources[i]->start);
    }
    PMPU_AHPB4_BASE = ioremap(pi3led_resources[0]->start ,resources_size[0]);
    PGPIOI_MODER = ioremap(pi3led_resources[1]->start ,resources_size[1]);
    PGPIOI_OTYPER = ioremap(pi3led_resources[2]->start ,resources_size[2]);
    PGPIOI_OSPEEDR = ioremap(pi3led_resources[3]->start ,resources_size[3]);
    PGPIOI_PUPDR = ioremap(pi3led_resources[4]->start ,resources_size[4]);
    PGPIOI_IDR = ioremap(pi3led_resources[5]->start ,resources_size[5]);
    PGPIOI_ODR = ioremap(pi3led_resources[6]->start ,resources_size[6]);
    PGPIOI_BSRR = ioremap(pi3led_resources[7]->start ,resources_size[7]);
#else
    PMPU_AHPB4_BASE = ioremap(RCC_MP_S_AHB4ENSETR ,4);
    PGPIOI_MODER    = ioremap(GPIOI_MODER ,4);
    PGPIOI_OTYPER   = ioremap(GPIOI_OTYPER ,4);
    PGPIOI_OSPEEDR  = ioremap(GPIOI_OSPEEDR ,4);
    PGPIOI_PUPDR    = ioremap(GPIOI_PUPDR ,4);
    PGPIOI_IDR      = ioremap(GPIOI_IDR ,4);
    PGPIOI_ODR      = ioremap(GPIOI_ODR ,4);
    PGPIOI_BSRR     = ioremap(GPIOI_BSRR ,4);
#endif
    /* 初始化寄存器 */
    /* 使能GPIOI时钟 */
    val = readl(PMPU_AHPB4_BASE);
    val &= ~(0x01 << 8);
    val |= (0x01 << 8);
    /* 端口模式寄存器：设置PI3为通用输出模式 */
    val = readl(PGPIOI_MODER);
    val &= ~(0x03 << 3);  
    val |= (0x01 << 3);     
    writel(val ,PGPIOI_MODER);
    /* 端口输出寄存器：设置PI3推挽模式 */
    val = readl(PGPIOI_OTYPER);
    val &= ~(0x01 << 3);  
    val |= (0x00 << 3);        
    writel(val ,PGPIOI_OTYPER);
    /* 端口输出速度寄存器：设置PI3为高速 */
    val = readl(PGPIOI_OSPEEDR);
    val &= ~(0x03 << 3);
    val |= (0x02 << 3);
    writel(val ,PGPIOI_OSPEEDR);
    /* 端口上拉/下拉寄存器：设置PI3为上拉 */
    val = readl(PGPIOI_PUPDR);
    val &= ~(0x03 << 3); 
    val |= (0x01 << 3);
    writel(val ,PGPIOI_PUPDR);
    /* 端口位设置/复位寄存器：PI3端口输出低电平 */
    val = readl(PGPIOI_BSRR);
    val &= ~(0x01 << 19); 
    val |= (0x01 << 19);
    writel(val ,PGPIOI_BSRR);

    ret = _abs_k_key();
    if(ret == -ERR_DEV_ID)
        goto err_dev_id;
    else if(ret == -ERR_DEV_CDEV)
        goto err_dev_cdev;
    else if(ret == -ERR_DEV_CLS)
        goto err_dev_cls;
    else if(ret == -ERR_DEV_DEV)
        goto err_dev_dev;
    pr_notice("%s dev abs k ok\r\n" ,LEDDEV_NAME);
    return 0;

err_dev_dev:
    device_destroy(pi3led_dev.cls ,pi3led_dev.id);
err_dev_cls:
    class_destroy(pi3led_dev.cls);
err_dev_cdev:
    cdev_del(&pi3led_dev.cdv);
err_dev_id:
    unregister_chrdev_region(pi3led_dev.id ,LEDDEV_CNT);
    pi3led_unmap();
    pr_err("%s init error\r\n" , LEDDEV_NAME);
    return -ENAVAIL;
}
/* 设备移除或驱动卸载时调用的回调函数 */
static int pi3led_remove(struct platform_device *dev){
    device_destroy(pi3led_dev.cls ,pi3led_dev.id);
    class_destroy(pi3led_dev.cls);
    cdev_del(&pi3led_dev.cdv);
    unregister_chrdev_region(pi3led_dev.id ,LEDDEV_CNT);
    pi3led_unmap();
    pr_notice("%s unload finished\r\n" ,LEDDEV_NAME);
    return 0;
}
/*----------------------------------------------------------------------------------*/
/*
* platform驱动结构体
*/
static struct platform_driver pi3led_drv = {
    .probe = pi3led_probe,
    .remove = pi3led_remove,
    .driver = {
        .name = "stm32mp135_led_pi3",
    }
};
/*----------------------------------------------------------------------------------*/
static int __init pi3led_drv_init(void){
    /* 向Linux内核注册一个platform驱动 */
    return platform_driver_register(&pi3led_drv);
}

static void __exit pi3led_drv_exit(void){
    /* Linux内核卸载一个platform驱动 */
    platform_driver_unregister(&pi3led_drv);
}
/*----------------------------------------------------------------------------------*/
module_init(pi3led_drv_init);
module_exit(pi3led_drv_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("baotou");
MODULE_INFO(intree, "Y");