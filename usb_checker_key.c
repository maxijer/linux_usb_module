#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/usb.h>
#include <linux/fs.h>
#include <linux/buffer_head.h>
 
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("USB Storage Scanner for Specific Word");
MODULE_VERSION("1.0");
 
/* Параметр: слово для поиска */
static char *search_word = "SecretKeyword";
module_param(search_word, charp, 0444);
MODULE_PARM_DESC(search_word, "Keyword to search in USB storage");
 
/* Функция чтения первых байтов с USB-накопителя */
static int read_usb_data(struct block_device *bdev, char *buffer, size_t size)
{
    struct file *filp;
    mm_segment_t old_fs;
    loff_t pos = 0;
    int ret = 0;
 
    /* Открываем устройство */
    filp = filp_open(bdev->bd_disk->disk_name, O_RDONLY, 0);
    if (IS_ERR(filp)) {
        printk(KERN_ERR "USBScanner: Cannot open device\n");
        return PTR_ERR(filp);
    }
 
    /* Читаем данные */
    old_fs = get_fs();
    set_fs(KERNEL_DS);
    ret = kernel_read(filp, buffer, size, &pos);
    set_fs(old_fs);
 
    filp_close(filp, NULL);
    return ret;
}
 
/* Функция поиска слова в буфере */
static int search_in_buffer(const char *buffer, size_t size, const char *word)
{
    int i;
    size_t word_len = strlen(word);
 
    for (i = 0; i <= size - word_len; i++) {
        if (memcmp(buffer + i, word, word_len) == 0)
            return 1; /* Найдено */
    }
    return 0; /* Не найдено */
}
 
/* Функция обработки каждого USB-устройства */
static int usb_storage_scan(struct usb_device *udev)
{
    struct block_device *bdev;
    char buffer[1024]; /* Читаем первые 1024 байта */
    int ret;
 
    /* Ищем блоковое устройство */
    bdev = blkdev_get_by_path("/dev/sdb1", FMODE_READ, NULL);
    if (IS_ERR(bdev)) {
        printk(KERN_ERR "USBScanner: Cannot get block device\n");
        return PTR_ERR(bdev);
    }
 
    /* Читаем данные */
    memset(buffer, 0, sizeof(buffer));
    ret = read_usb_data(bdev, buffer, sizeof(buffer));
    blkdev_put(bdev, FMODE_READ);
 
    if (ret < 0) {
        printk(KERN_ERR "USBScanner: Read error\n");
        return ret;
    }
 
    /* Ищем слово */
    if (search_in_buffer(buffer, sizeof(buffer), search_word)) {
        printk(KERN_INFO "USBScanner: Found '%s' on USB storage!\n", search_word);
    } else {
        printk(KERN_INFO "USBScanner: Word '%s' not found\n", search_word);
    }
 
    return 0;
}
 
/* Инициализация модуля */
static int __init usb_scanner_init(void)
{
    printk(KERN_INFO "USBScanner: Module Loaded, searching for: %s\n", search_word);
    return usb_storage_scan(NULL);
}
 
/* Выгрузка модуля */
static void __exit usb_scanner_exit(void)
{
    printk(KERN_INFO "USBScanner: Module Unloaded\n");
}
 
module_init(usb_scanner_init);
module_exit(usb_scanner_exit);
