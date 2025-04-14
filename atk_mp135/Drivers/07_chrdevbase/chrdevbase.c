#include <linux/types.h>
#include <linux/kernel.h> 
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
/***************************************************************
 *Copyright © ALIENTEK Co., Ltd. 1998-2029. All rights reserved.
 * 文件名 : chrdevbase.c
 * 作者 : baotou
 * 版本 : V1.0
 * 描述 : chrdevbase 驱动文件。
 * 日志 : 初版 V1.0 2025/3/18
***************************************************************/

#define CHRDEVBASE_NAME         "chrdevbase"    /* 设备名字 */
#define CHRDEVBASE_MAJOR        200             /* 主设备号 */
static char writebuf[100];                      /* kernel写缓冲区 */
static char readbuf[100];                       /* kernel读缓冲区 */
static char kerneldata[] = {"kernel Data!"};


/*
* @function_name : chrdevbase_open
* @description   : 打开设备
* @param  pinode : 传递给驱动的 inode(节点，在文件创建时创建，包含了文件的元数据)
* @param  pfilp  : 设备文件， file 结构体有个叫做 private_data
*                  (用于存储文件的私有数据，由文件操作函数使用)的成员变量
*                  一般在 open 的时候将 private_data 指向设备结构体。
* @return        : 0 
*/
static int chrdevbase_open(struct inode *pinode ,struct file *pfile)
{ 
    return 0;    
}
/*
* @function_name : chrdevbase_read
* @description   : 从设备读取数据
* @param - pfilp : 要打开的设备文件(文件描述符)
* @param - buf   : 返回给用户空间的数据缓冲区
* @param - cnt   : 要读取的数据长度
* @param - offt  : 相对于文件首地址的偏移
* @return        : 0 -- 成功；-1 -- 失败
*/
static ssize_t chrdevbase_read(struct file *pfile ,char __user *buf ,size_t cnt ,loff_t *offt)
{
    int retvalue = 0;
    memcpy(readbuf ,kerneldata ,sizeof(kerneldata));
    /* 读取内核空间的数据 */
    retvalue = copy_to_user(buf ,readbuf ,cnt);
    if(retvalue == 0){
        return 0;
    }
    return -1;
}
/*
* @function_name : chrdevbase_write
* @description   : 向设备写数据
* @param - filp  : 设备文件，表示打开的文件描述符
* @param - buf   : 要写给设备写入的数据
* @param - cnt   : 要写入的数据长度
* @param - offt  : 相对于文件首地址的偏移
* @return        : 0 -- 成功；-1 -- 失败
*/
static ssize_t chrdevbase_write(struct file *pfile ,const char __user *buf ,size_t cnt ,loff_t *offt)
{
    size_t retvalue = 0;
    /* 向内核空间写入数据 */
    retvalue = copy_from_user(writebuf ,buf ,cnt);
    if(retvalue == 0)
    {
        printk(KERN_NOTICE "Write KernelData Success\r\n");
        printk(KERN_NOTICE "Kernel Writebuf = %s\r\n",writebuf);
        return 0;
    }
    return -1;
}
/*
* @function_name : chrdevbase_release
* @description   : 关闭/释放设备
* @param  pinode : 传递给驱动的 inode(节点，在文件创建时创建，包含了文件的元数据)
* @param  pfilp  : 设备文件，表示打开的文件描述符
* @return        : 0
*/
static int chrdevbase_release(struct inode *pinode,struct file *pfilp)
{
    return 0;
}

/* 加入字符设备注册和注销 */
/* 初始化chrdevbase_file */
struct file_operations chrdevbase_file = 
{
    .owner = THIS_MODULE,
    .open = chrdevbase_open,
    .write = chrdevbase_write,
    .read = chrdevbase_read,
    .release = chrdevbase_release,
};
/*
* @function_name : chrdevbase_init
* @description   : 驱动入口
* @return        : 0 - 成功/ -1 - 失败
*/
 static int __init chrdevbase_init(void)
 {
    /* 注册字符驱动 */
    int retvalue = register_chrdev(CHRDEVBASE_MAJOR ,CHRDEVBASE_NAME ,&chrdevbase_file);
    if(retvalue < 0)
    {
        printk(KERN_ALERT "Chrdevbase Driver Register Failed\r\n");
        return -1;
    }
    printk(KERN_INFO "Chrdevbase Driver Register Success\r\n");
    return 0;
 }
/*
* @function_name : chrdevbase_exit
* @description   : 驱动出口
* @return        : 无
*/
static void __exit chrdevbase_exit(void)
{
   /* 注销字符驱动 */
   unregister_chrdev(CHRDEVBASE_MAJOR ,CHRDEVBASE_NAME);
   printk(KERN_INFO "Chrdevbase Driver LogOut\r\n");
}
/* 将上面两个函数指定为驱动的入口和出口函数 */
module_init(chrdevbase_init);
module_exit(chrdevbase_exit);  
/*
* LICENSE 和作者信息
*/
MODULE_LICENSE("GPL");
MODULE_AUTHOR("ALIENTEK");
MODULE_INFO(intree, "Y");
