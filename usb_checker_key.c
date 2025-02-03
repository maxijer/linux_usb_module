#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/usb.h>
#include <linux/fs.h>
#include <linux/buffer_head.h>
#include <linux/genhd.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("USB Storage Scanner for Specific Word");
MODULE_VERSION("1.1");

static char *search_word = "SecretKeyword";
module_param(search_word, charp, 0444);
MODULE_PARM_DESC(search_word, "Keyword to search in USB storage");

/* Функция поиска слова в буфере */
static int search_in_buffer(const char *buffer, size_t size, const char *word)
{
    size_t word_len = strlen(word);
    size_t i;

    for (i = 0; i <= size - word_len; i++) {
        if (memcmp(buffer + i, word, word_len) == 0)
            return 1; /* Найдено */
    }
    return 0; /* Не найдено */
}

/* Функция чтения первых байтов USB-накопителя */
static int read_usb_data(const char *dev_path, char *buffer, size_t size)
{
    struct file *filp;
    loff_t pos = 0;
    ssize_t ret;

    /* Открываем устройство */
    filp = filp_open(dev_path, O_RDONLY, 0);
    if (IS_ERR(filp)) {
        printk(KERN_ERR "USBScanner: Cannot open %s\n", dev_path);
        return PTR_ERR(filp);
    }

    /* Читаем данные */
    ret = kernel_read(filp, buffer, size, &pos);
    filp_close(filp, NULL);

    if (ret < 0) {
        printk(KERN_ERR "USBScanner: Read error from %s\n", dev_path);
        return ret;
    }

    return 0;
}

/* Функция сканирования USB-накопителя */
static int scan_usb_storage(void)
{
    char buffer[1024]; /* Читаем первые 1024 байта */
    const char *usb_device = "/dev/sdb"; /* Укажите правильный путь */

    memset(buffer, 0, sizeof(buffer));

    /* Читаем данные с флешки */
    if (read_usb_data(usb_device, buffer, sizeof(buffer)) < 0)
        return -1;

    /* Ищем слово */
    if (search_in_buffer(buffer, sizeof(buffer), search_word)) {
        printk(KERN_INFO "USBScanner: Found '%s' on USB storage!\n", search_word);
    } else {
        printk(KERN_INFO "USBScanner: Word '%s' not found on USB storage.\n", search_word);
    }

    return 0;
}

/* Функция инициализации модуля */
static int __init usb_scanner_init(void)
{
    printk(KERN_INFO "USBScanner: Module Loaded, searching for: %s\n", search_word);
    return scan_usb_storage();
}

/* Функция выгрузки модуля */
static void __exit usb_scanner_exit(void)
{
    printk(KERN_INFO "USBScanner: Module Unloaded\n");
}

module_init(usb_scanner_init);
module_exit(usb_scanner_exit);
