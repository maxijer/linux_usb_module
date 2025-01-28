#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/usb.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/proc_fs.h>
#include <linux/buffer_head.h>

#define KEYWORD "MY_USB_KEY"
#define HEADER_SIZE 512

static struct proc_dir_entry *proc_file;

static int check_usb_devices(void)
{
    char dev_path[16];
    struct file *filp;
    mm_segment_t oldfs;
    char buf[HEADER_SIZE];
    int found = 0;
    int i;

    for (i = 1; i <= 5; i++) {  // Проверяем /dev/sda - /dev/sde
        snprintf(dev_path, sizeof(dev_path), "/dev/sd%c", 'a' + i - 1);

        filp = filp_open(dev_path, O_RDONLY, 0);
        if (IS_ERR(filp)) {
            printk(KERN_INFO "usb_key_checker: Пропускаем %s (не найден)\n", dev_path);
            continue;
        }

        oldfs = get_fs();
        set_fs(KERNEL_DS);

        if (vfs_read(filp, buf, HEADER_SIZE, &filp->f_pos) > 0) {
            if (strnstr(buf, KEYWORD, HEADER_SIZE)) {
                printk(KERN_INFO "usb_key_checker: Ключ найден на %s!\n", dev_path);
                found = 1;
            }
        }

        set_fs(oldfs);
        filp_close(filp, NULL);

        if (found)
            break;
    }

    return found;
}

static ssize_t usb_key_read(struct file *file, char __user *user_buffer, size_t count, loff_t *position)
{
char msg[64];
int found = check_usb_devices();
int len = snprintf(msg, sizeof(msg), "USB Key: %s\n", found ? "FOUND" : "NOT FOUND");

if (*position >= len)  // Если уже прочитали всё сообщение
return 0;

if (copy_to_user(user_buffer, msg, len))
return -EFAULT;

*position += len;
return len;
}

static struct proc_ops proc_fops = {
        .proc_read = usb_key_read,
};

static int __init usb_key_checker_init(void)
{
    proc_file = proc_create("usb_key_checker", 0, NULL, &proc_fops);
    if (!proc_file) {
        printk(KERN_ERR "usb_key_checker: Ошибка создания /proc/usb_key_checker\n");
        return -ENOMEM;
    }

    printk(KERN_INFO "usb_key_checker: Загружен\n");
    return 0;
}

static void __exit usb_key_checker_exit(void)
{
    remove_proc_entry("usb_key_checker", NULL);
    printk(KERN_INFO "usb_key_checker: Выгружен\n");
}

module_init(usb_key_checker_init);
module_exit(usb_key_checker_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("USB Key Check Kernel Module");
