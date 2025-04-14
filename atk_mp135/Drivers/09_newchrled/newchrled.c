#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/gpio.h>
#include <linux/cdev.h>
#include <linux/kdev_t.h>
#include <asm/mach/map.h>
#include <asm/uaccess.h>
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
#define NEWCHRLED_CNT           (0x01)
#define NEWCHRLED_NAME          ("newchrled")
/* 寄存器物理地址 */
#define PERIPH_BASE_ADDR        (0x40000000)                            /*外设寄存器基地址*/
#define MPU_AHPB4_BASE_ADDR     (PERIPH_BASE_ADDR + 0x10000000)         /*MPU访问AHB4（总连接的外设（如GPIO、RCC））外设的基地址*/
#define RCC_BASE_ADDR           (MPU_AHPB4_BASE_ADDR + 0x00)            /*RCC时钟控制器基地址，用于管理系统时钟*/
#define RCC_MP_S_AHB4ENSETR     (RCC_BASE_ADDR + 0XA28)                 /*AHB4 外设时钟使能寄存器:控制GPIO端口*/
#define GPIOI_BASE              (MPU_AHPB4_BASE_ADDR + 0xA000)          /*GPIOI 外设基地址*/
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
/******************************************************************/
/*变量*************************************************************/
/* 映射后的寄存器虚拟地址指针结构体 */
typedef struct
{
    /* __iomem: 是一种标记，表明指针是指向I/O内存区域。它是一个编译器注解，
    用来让编译器进行检查，确保正确处理这些特殊的内存区域*/
    void __iomem* P_MPU_AHPB4_BASE;
    void __iomem* P_MODER;
    void __iomem* P_OTYPER;
    void __iomem* P_OSPEEDR;
    void __iomem* P_PUPDR;
    void __iomem* P_IDR;
    void __iomem* P_ODR;
    void __iomem* P_BSRR;
}__GPIOx_REGVA;
static __GPIOx_REGVA* GPIOx_REGVA;
/* newdev设备结构体 */
struct newdev
{
	dev_t devid;			/* 设备id */
	struct cdev _cdev;      /* 设备信息 */
	struct class *_class;   /* 设备类 */
	struct device *_device; /* 设备 */
	int major;              /* 主设备号 */
	int minor;              /* 次设备号 */
};
struct newdev newchrled;    /* led设备 */
/******************************************************************/
/*驱动函数**********************************************************/
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
        writel(val ,GPIOx_REGVA->P_BSRR);
    }
    /* 输出低电平 */
    else
    {
        /* BSRR寄存器的高15位BR[15：0]用于引脚复位为0 */
        val |= (1 << (3 + 0x10));
        writel(val ,GPIOx_REGVA->P_BSRR);
    }
}
/*
 * @function_name       : GPIOxREGVA_unmap
 * @description         : 取消映射
 * @param - GPIOI_REGVA : 寄存器结构体
 * @return              : 无
*/
void GPIOxREGVA_unmap(void)
{
    if(GPIOx_REGVA == NULL)
    {
        printk(KERN_WARNING "struct GPIOx_REGVA is null , skipping iounmap.\r\n");
        return;
    }
    iounmap(GPIOx_REGVA->P_MPU_AHPB4_BASE);
    iounmap(GPIOx_REGVA->P_MODER);
    iounmap(GPIOx_REGVA->P_OTYPER);
    iounmap(GPIOx_REGVA->P_OSPEEDR);
    iounmap(GPIOx_REGVA->P_PUPDR);
    iounmap(GPIOx_REGVA->P_IDR);
    iounmap(GPIOx_REGVA->P_BSRR);
}
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
	pfile->private_data = &newchrled;
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
    int retvalue = 0;
    u8 io_sta;
    if(cnt < 1)
    {
        return -1;
    }
    /* 获取单个io口的值 */
    io_sta = (readl(GPIOx_REGVA->P_IDR) >> 3) & 0x01;
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
    /* 向内核空间写入数据 */
    retvalue = copy_from_user(&wdata ,buf ,cnt);
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
static int __init newchrled_init(void)
{
    u32 val;
/*0.分配内存*--------------------------------------------------------------------*/
    /* GPIOx_REGVA 是一个指针类型，sizeof(GPIOx_REGVA) 会返回指针大小（通常是4或8字节）。
    如果 GPIOx_REGVA 是一个结构体指针，你应该使用结构体的大小，而不是指针的大小:
    GPIOx_REGVA = kzalloc(sizeof(*GPIOx_REGVA), GFP_KERNEL); */
    GPIOx_REGVA = (__GPIOx_REGVA*)kzalloc(sizeof(*GPIOx_REGVA) ,GFP_KERNEL);
    if(GPIOx_REGVA == NULL)
    {
        pr_err("GPIOx_REGVA no memory allocated\r\n");
        return -1;
    }

/*1.寄存器地址映射----------------------------------------------------------------*/
    GPIOx_REGVA->P_MPU_AHPB4_BASE = ioremap(RCC_MP_S_AHB4ENSETR ,4);
    GPIOx_REGVA->P_MODER    = ioremap(GPIOI_MODER ,4);
    GPIOx_REGVA->P_OTYPER   = ioremap(GPIOI_OTYPER ,4);
    GPIOx_REGVA->P_OSPEEDR  = ioremap(GPIOI_OSPEEDR ,4);
    GPIOx_REGVA->P_PUPDR    = ioremap(GPIOI_PUPDR ,4);
    GPIOx_REGVA->P_IDR      = ioremap(GPIOI_IDR ,4);
    GPIOx_REGVA->P_ODR      = ioremap(GPIOI_ODR ,4);
    GPIOx_REGVA->P_BSRR     = ioremap(GPIOI_BSRR ,4);
    /* 判断每一个寄存器是否映射成功 */
    if((!GPIOx_REGVA->P_MPU_AHPB4_BASE) || (!GPIOx_REGVA->P_MODER) ||(!GPIOx_REGVA->P_OTYPER) 
        || (!GPIOx_REGVA->P_OSPEEDR) || (!GPIOx_REGVA->P_PUPDR)  || (!GPIOx_REGVA->P_IDR)  
          ||(!GPIOx_REGVA->P_ODR) || (!GPIOx_REGVA->P_BSRR))
    {
        pr_err("register mapping failed\r\n");
        goto  del_unmap;
    }

/*2.初始化io口-------------------------------------------------------------------*/
    /* 使能GPIOI时钟 */
    val = readl(GPIOx_REGVA->P_MPU_AHPB4_BASE);
    val &= ~(0x01 << 8);
    val |= (0x01 << 8);
    /* 端口模式寄存器：设置PI3为通用输出模式 */
    val = readl(GPIOx_REGVA->P_MODER);
    val &= ~(0x03 << 3);  
    val |= (0x01 << 3);     
    writel(val ,GPIOx_REGVA->P_MODER);
    /* 端口输出寄存器：设置PI3推挽模式 */
    val = readl(GPIOx_REGVA->P_OTYPER);
    val &= ~(0x01 << 3);  
    val |= (0x00 << 3);        
    writel(val ,GPIOx_REGVA->P_OTYPER);
    /* 端口输出速度寄存器：设置PI3为高速 */
    val = readl(GPIOx_REGVA->P_OSPEEDR);
    val &= ~(0x03 << 3);
    val |= (0x02 << 3);
    writel(val ,GPIOx_REGVA->P_OSPEEDR);
    /* 端口上拉/下拉寄存器：设置PI3为上拉 */
    val = readl(GPIOx_REGVA->P_PUPDR);
    val &= ~(0x03 << 3); 
    val |= (0x01 << 3);
    writel(val ,GPIOx_REGVA->P_PUPDR);
    /* 端口位设置/复位寄存器：PI3端口输出低电平 */
    val = readl(GPIOx_REGVA->P_BSRR);
    val &= ~(0x01 << 19); 
    val |= (0x01 << 19);
    writel(val ,GPIOx_REGVA->P_BSRR);
/*3.注册字符设备驱动---------------------------------------------------------------*/
/*3.1 创建字符设备 */
    /* 已经定义设备号 */
    if(newchrled.major)
    {
        /* 将主设备号和次设备号组合成一个dev_t类型的设备号 */
        newchrled.devid = MKDEV(newchrled.major ,0);
        /* 注册固定设备号 */
        if(register_chrdev_region(newchrled.devid ,NEWCHRLED_CNT ,NEWCHRLED_NAME) < 0)
        {
            /* 注册失败 */
            pr_err("%s regsiter failed\r\n" ,NEWCHRLED_NAME);
            goto del_unmap;           
        }
    }
    /* 没有定义设备号 */
    else
    {
        /* 自动分配设备号 */
        if(alloc_chrdev_region(&newchrled.devid ,0 ,NEWCHRLED_CNT ,NEWCHRLED_NAME) < 0)
        {
            /* 注册失败 */
            pr_err("%s regsiter failed\r\n" ,NEWCHRLED_NAME);
            goto del_unmap;  
        }
        /* 获取主设备号 */
        newchrled.major = MAJOR(newchrled.devid); /* 此代码可以省略 */
        /* 获取次设备号 */
        newchrled.minor = MINOR(newchrled.devid); /* 此代码可以省略 */
        pr_notice("%s: major = %d , minor = %d\r\n",NEWCHRLED_NAME ,newchrled.major ,newchrled.minor);
    }
    pr_notice("%s regsiter devid finished\r\n" ,NEWCHRLED_NAME);


/*3.2 创建字符设备小管家cdev */
    /* 将newchrled._cdev字符设备的 owner 成员指向当前驱动模块 */
    newchrled._cdev.owner = THIS_MODULE;
    /* 初始化cdev */
    cdev_init(&newchrled._cdev ,&GPIOI_PI3_led_file);
    /* 向linux系统添加字符设备 */
    if(cdev_add(&newchrled._cdev ,newchrled.devid ,NEWCHRLED_CNT) < 0)
    {
        pr_err("%s cdev failed to add to linux\r\n",NEWCHRLED_NAME);
        goto del_devid;
    }
    pr_notice("%s cdev finished to add to linux\r\n",NEWCHRLED_NAME);

/*3.3 创建类:方便用户空间访问设备 */
    newchrled._class = class_create(THIS_MODULE ,NEWCHRLED_NAME);
    if(IS_ERR(newchrled._class))
    {
        pr_err("%s created class failed\r\n",NEWCHRLED_NAME);
        goto del_cdev;
    }
    pr_notice("%s created class finished\r\n",NEWCHRLED_NAME);

/*3.4 创建设备 */
    newchrled._device = device_create(newchrled._class ,NULL ,newchrled.devid ,NULL ,NEWCHRLED_NAME);
    if(IS_ERR(newchrled._device))
    {
        pr_err("%s created device failed\r\n",NEWCHRLED_NAME);
        goto del_class;
    }
    pr_notice("%s created device finished\r\n",NEWCHRLED_NAME);

    return 0;
/*------------------------------------------------------------------------------*/
/* 注销类 */
del_class:
    class_destroy(newchrled._class);
/* 注销设备 */
del_cdev:
    cdev_del(&newchrled._cdev);
/* 注销设备号 */
del_devid:
    unregister_chrdev_region(newchrled.devid ,NEWCHRLED_CNT);
/* 取消映射 */
del_unmap:
    GPIOxREGVA_unmap();
    kfree(GPIOx_REGVA);
    return -1;
}
/*
 * @description	: 驱动出口函数
 * @param 		: 无
 * @return 		: 无
 */
static void __exit newchrled_exit(void)
{
    device_destroy(newchrled._class ,newchrled.devid);
    class_destroy(newchrled._class);
    cdev_del(&newchrled._cdev);
    unregister_chrdev_region(newchrled.devid ,NEWCHRLED_CNT);
    GPIOxREGVA_unmap();
    kfree(GPIOx_REGVA);
    pr_notice("%s logout finished\r\n",NEWCHRLED_NAME);
}

module_init(newchrled_init);
module_exit(newchrled_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("baotou");
MODULE_INFO(intree,"Y");
