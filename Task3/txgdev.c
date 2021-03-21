#include <linux/module.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <asm/io.h>
#include <linux/slab.h>
#include <linux/version.h>
#if  LINUX_VERSION_CODE  >  KERNEL_VERSION(3,  3,  0)
    #include <asm/switch_to.h>
#else
    #include <asm/system.h>
#endif
#include <asm/uaccess.h>

#ifndef TXGDEV_MAJOR
#define TXGDEV_MAJOR 150    /* 预设的主设备号 */
#endif

#ifndef TXGDEV_NR_DEVS
#define TXGDEV_NR_DEVS 1    /* 设备数 */
#endif

#ifndef TXGDEV_SIZE
#define TXGDEV_SIZE 4096
#endif

/* 设备描述结构体 */
struct txg_dev {
    char *data;
    unsigned long size;       
};

static int txg_major = TXGDEV_MAJOR;

module_param(txg_major, int, S_IRUGO);

struct txg_dev *txg_devp; /* 设备结构体指针 */

struct cdev cdev;

/* 文件打开函数 */
int txg_open(struct inode *inode, struct file *filp) {
    struct txg_dev *dev;
    
    /* 获取次设备号 */
    int num = MINOR(inode->i_rdev);

    if (num >= TXGDEV_NR_DEVS)
        return -ENODEV;
    dev = &txg_devp[num];
    
    /* 将设备描述结构指针赋值给文件私有数据指针 */
    filp->private_data = dev;
    
    return 0;
}

/* 文件释放函数 */
int txg_release(struct inode *inode, struct file *filp) {
    return 0;
}

/* 读函数 */
static ssize_t txg_read(struct file *filp, char __user *buf, size_t size, loff_t *ppos) {
    unsigned long p =  *ppos;       /* 记录文件指针偏移位置 */
    unsigned int count = size;      /* 记录需要读取的字节数 */ 
    int ret = 0;                    /* 返回值 */
    struct txg_dev *dev = filp->private_data;   /* 获得设备结构体指针 */
    
    /* 判断读位置是否有效 */
    if(p >= TXGDEV_SIZE)            /* 要读取的偏移大于设备的内存空间 */
        return 0;
    if(count > TXGDEV_SIZE - p)     /* 要读取的字节大于设备的剩余内存空间 */ 
        count = TXGDEV_SIZE - p;

    /* 读数据到用户空间: 内核空间->用户空间交换数据 */  
    if(copy_to_user(buf, (void*)(dev->data + p), count)) {
        ret =  - EFAULT;
    }
    else {
        *ppos += count;
        ret = count;
        printk(KERN_INFO "read %d bytes(s) from %ld\n", count, p);
    }
    
    return ret;
}

/* 写函数 */
static ssize_t txg_write(struct file *filp, const char __user *buf, size_t size, loff_t *ppos) {
    unsigned long p =  *ppos;
    unsigned int count = size;
    int ret = 0;
    struct txg_dev *dev = filp->private_data;   /* 获得设备结构体指针 */
    
    /* 分析和获取有效的写长度 */
    if(p >= TXGDEV_SIZE)
        return 0;
    if(count > TXGDEV_SIZE - p)             /* 要写入的字节大于设备的剩余内存空间 */
        count = TXGDEV_SIZE - p;
    
    /* 清空设备中上一次写入的数据 */
    if(p == 0) {
        memset((void*)(dev->data), 0, TXGDEV_SIZE);
    }

    /* 从用户空间写入数据 */
    if(copy_from_user(dev->data + p, buf, count))
        ret =  - EFAULT;
    else {
        *ppos += count;     /* 增加偏移位置 */  
        ret = count;        /* 返回实际的写入字节数 */ 
        printk(KERN_INFO "written %d bytes(s) from %ld\n", count, p);
    }
    
    return ret;
}

/* seek文件定位函数 */
static loff_t txg_llseek(struct file *filp, loff_t offset, int whence)
{ 
    loff_t newpos;

    switch(whence) {
      case 0: /* SEEK_SET */       /* 相对文件开始位置偏移 */ 
        newpos = offset;           /* 更新文件指针位置 */
        break;

      case 1: /* SEEK_CUR */
        newpos = filp->f_pos + offset;    
        break;

      case 2: /* SEEK_END */
        newpos = TXGDEV_SIZE -1 + offset;
        break;

      default: /* can't happen */
        return -EINVAL;
    }
    if ((newpos<0) || (newpos>TXGDEV_SIZE))
        return -EINVAL;
    
    filp->f_pos = newpos;
    return newpos;
}

/* 文件操作结构体 */
static const struct file_operations txg_fops = {
    .owner = THIS_MODULE,
    .llseek = txg_llseek,
    .read = txg_read,
    .write = txg_write,
    .open = txg_open,
    .release = txg_release,
};

/* 设备驱动模块加载函数 */
static int txgdev_init(void) {
    int result;
    int i;

    dev_t devno = MKDEV(txg_major, 0);

    /* 申请设备号，当xxx_major不为0时，表示静态指定；当为0时，表示动态申请 */ 
    /* 静态申请设备号 */
    if(txg_major)
        result = register_chrdev_region(devno, TXGDEV_NR_DEVS, "txgdev");
    else {  /* 动态分配设备号 */
        result = alloc_chrdev_region(&devno, 0, TXGDEV_NR_DEVS, "txgdev");
        txg_major = MAJOR(devno);   /* 获得申请的主设备号 */
    }  
  
    if(result < 0)
        return result;

    /* 初始化cdev结构，并传递file_operations结构指针 */ 
    cdev_init(&cdev, &txg_fops);    
    cdev.owner = THIS_MODULE;       /* 指定所属模块 */
    cdev.ops = &txg_fops;
  
    /* 注册字符设备 */
    cdev_add(&cdev, MKDEV(txg_major, 0), TXGDEV_NR_DEVS);
   
    /* 为设备描述结构分配内存 */
    txg_devp = kmalloc(TXGDEV_NR_DEVS * sizeof(struct txg_dev), GFP_KERNEL);
    if(!txg_devp) {     /* 申请失败 */
        result =  - ENOMEM;
        goto fail_malloc;
    }
    memset(txg_devp, 0, sizeof(struct txg_dev));
  
    /* 为设备分配内存 */
    for (i=0; i < TXGDEV_NR_DEVS; i++) {
        txg_devp[i].size = TXGDEV_SIZE;
        txg_devp[i].data = kmalloc(TXGDEV_SIZE, GFP_KERNEL);
        memset(txg_devp[i].data, 0, TXGDEV_SIZE);
    }
    
    return 0;

fail_malloc: 
    unregister_chrdev_region(devno, 1);
    
    return result;
}

/* 模块卸载函数 */
static void txgdev_exit(void) {
    cdev_del(&cdev);        /* 注销设备 */
    kfree(txg_devp);        /* 释放设备结构体内存 */
    unregister_chrdev_region(MKDEV(txg_major, 0), TXGDEV_NR_DEVS);  /* 释放设备号 */
}

MODULE_AUTHOR("TXG");
MODULE_LICENSE("GPL");

module_init(txgdev_init);
module_exit(txgdev_exit);