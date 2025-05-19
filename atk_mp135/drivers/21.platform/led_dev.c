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
/* 寄存器物理地址 */
#define PERIPH_BASE_ADDR        (0x40000000)                            /*外设寄存器基地址*/
#define MPU_AHPB4_BASE_ADDR     (PERIPH_BASE_ADDR + 0x10000000)         /*MPU访问AHB4（总连接的外设（如GPIO、RCC））外设的基地址*/
#define RCC_BASE_ADDR           (MPU_AHPB4_BASE_ADDR + 0x00)            /*RCC时钟控制器基地址，用于管理系统时钟*/
#define RCC_MP_S_AHB4ENSETR     (RCC_BASE_ADDR + 0XA28)                 /*AHB4 外设时钟使能寄存器:控制GPIO端口*/
#define GPIOI_BASE              (MPU_AHPB4_BASE_ADDR + 0xA000)          /*GPIOI 外设基地址*/
#define GPIOI_OFFEST_ADDR       (0x04)                                  /*GPIOI 寄存器的地址偏移量*/
#if 1
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
#endif
/*----------------------------------------------------------------------------------*/
/**
 * struct struct_name - led的platform设备资源（寄存器）结构体
 * @member1: 
 * @member2: 
 */
static struct resource pi3led_resource[] = {
    [0] = {
        .start = RCC_MP_S_AHB4ENSETR,
        .end = (RCC_MP_S_AHB4ENSETR + GPIOI_OFFEST_ADDR - 1),
        .flags = IORESOURCE_MEM
    },
    [1] = {
        .start = GPIOI_MODER,
        .end = (GPIOI_MODER + GPIOI_OFFEST_ADDR - 1),
        .flags = IORESOURCE_MEM
    },
    [2] = {
        .start = GPIOI_OTYPER,
        .end = (GPIOI_OTYPER + GPIOI_OFFEST_ADDR - 1),
        .flags = IORESOURCE_MEM
    },
    [3] = {
        .start = GPIOI_OSPEEDR,
        .end = (GPIOI_OSPEEDR + GPIOI_OFFEST_ADDR - 1),
        .flags = IORESOURCE_MEM
    },
    [4] = {
        .start = GPIOI_PUPDR,
        .end = (GPIOI_PUPDR + GPIOI_OFFEST_ADDR - 1),
        .flags = IORESOURCE_MEM
    },
    [5] = {
        .start = GPIOI_IDR,
        .end = (GPIOI_IDR + GPIOI_OFFEST_ADDR - 1),
        .flags = IORESOURCE_MEM
    },
    [6] = {
        .start = GPIOI_ODR,
        .end = (GPIOI_ODR + GPIOI_OFFEST_ADDR - 1),
        .flags = IORESOURCE_MEM
    },
    [7] = {
        .start = GPIOI_BSRR,
        .end = (GPIOI_BSRR + GPIOI_OFFEST_ADDR - 1),
        .flags = IORESOURCE_MEM
    }
};

/**
 * struct struct_name - led的platform设备结构体
 * @member1: 
 * @member2: 
 * 用于描述设备，如果支持设备树尽量不用次结构体描述
 */
static void pi3led_release(struct device *dev);
static struct platform_device pi3led_dev = {
    .name = "stm32mp135_led_pi3",
    .id = -1,
    .dev = {
        .release = pi3led_release
    },
    .num_resources = ARRAY_SIZE(pi3led_resource),
    .resource = pi3led_resource
};

/*----------------------------------------------------------------------------------*/
/**
 * @function_name - 释放platform设备回调函数	
 * @param1
 * @param2
 * @return
 */
static void pi3led_release(struct device *dev){
    pr_notice("platform-pi3led device released!\r\n");
}

/*----------------------------------------------------------------------------------*/
/**
 * @function_name - 驱动入口
 * @param1
 * @param2
 * @return
 */
static int __init pi3led_dev_init(void){
    return platform_device_register(&pi3led_dev);
}

/**
 * @function_name - 驱动出口
 * @param1
 * @param2
 * @return
 */
static void __exit pi3led_dev_exit(void){
    platform_device_unregister(&pi3led_dev);
}
/*----------------------------------------------------------------------------------*/
module_init(pi3led_dev_init);
module_exit(pi3led_dev_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("baotou");
MODULE_INFO(intree, "Y");


