#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/gpio.h>
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
*******************************************************************/
/*宏定义************************************************************/
/* LED */
#define LED_MAJOR               (201)
#define LED_NAME                ("LED_PI3")
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

    /* C90不支持变量在for语句的表达式中定义
    void __iomem* ptrs[] = 
    {
        GPIOx_REGVA->P_MPU_AHPB4_BASE,
        GPIOx_REGVA->P_MODER,
        GPIOx_REGVA->P_OTYPER,
        GPIOx_REGVA->P_OSPEEDR,
        GPIOx_REGVA->P_PUPDR,
        GPIOx_REGVA->P_IDR,
        GPIOx_REGVA->P_ODR,
        GPIOx_REGVA->P_BSRR
    }; 
    u8 i = 0;
    for(i = 0 ;i < ARRAY_SIZE(ptrs) ;i++)
    {
        if(ptrs[i])
        {
            iounmap(ptrs[i]);
        }
    }
    */
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
    .owner = THIS_MODULE,
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
static int __init pi3_led_init(void)
{
	int ret = 0;
    u32 val;
/*0.分配内存****************************************************************/
    GPIOx_REGVA = kzalloc(sizeof(GPIOx_REGVA) ,GFP_KERNEL);
    if(GPIOx_REGVA == NULL)
    {
        kfree(GPIOx_REGVA);
        printk(KERN_WARNING "GPIOx_REGVA no memory allocated\r\n");
        return -1;
    }
/*1.寄存器地址映射****************************************************************/
    GPIOx_REGVA->P_MPU_AHPB4_BASE = ioremap(RCC_MP_S_AHB4ENSETR ,4);
    GPIOx_REGVA->P_MODER    = ioremap(GPIOI_MODER ,4);
    GPIOx_REGVA->P_OTYPER   = ioremap(GPIOI_OTYPER ,4);
    GPIOx_REGVA->P_OSPEEDR  = ioremap(GPIOI_OSPEEDR ,4);
    GPIOx_REGVA->P_PUPDR    = ioremap(GPIOI_PUPDR ,4);
    GPIOx_REGVA->P_IDR      = ioremap(GPIOI_IDR ,4);
    GPIOx_REGVA->P_ODR      = ioremap(GPIOI_ODR ,4);
    GPIOx_REGVA->P_BSRR     = ioremap(GPIOI_BSRR ,4);
/*2.初始化io口*******************************************************************/
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
/*3.注册字符设备驱动**************************************************************/
	ret = register_chrdev(LED_MAJOR, LED_NAME, &GPIOI_PI3_led_file);
	if(ret < 0) {
		printk("register chrdev failed!\r\n");
        /* 取消映射 */
        GPIOxREGVA_unmap();
        /* 释放指针 */
        kfree(GPIOx_REGVA);
		return -1;
	}
    printk(KERN_INFO "register chrdev success\r\n");
	return 0;
}

/*
 * @description	: 驱动出口函数
 * @param 		: 无
 * @return 		: 无
 */
static void __exit pi3_led_exit(void)
{
    /* 取消映射 */
    GPIOxREGVA_unmap();
    /* 释放指针 */
    kfree(GPIOx_REGVA);
    printk(KERN_INFO "register chrdev log out\r\n");
	/* 注销字符设备驱动 */
	unregister_chrdev(LED_MAJOR, LED_NAME);
}
/******************************************************************/
/******************************************************************/
module_init(pi3_led_init);
module_exit(pi3_led_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("baotou");
MODULE_INFO(intree, "Y");

