#include <stdio.h>
#include <stdlib.h>

int main()
{
    FILE *f = fopen("/proc/usb_key_checker", "r");
    if (!f) {
        printf("Ошибка: модуль ядра не загружен\n");
        return 1;
    }

    char response[64];
    if (fgets(response, sizeof(response), f) != NULL) {
        printf("%s", response);
    } else {
        printf("Ошибка при чтении из модуля\n");
    }

    fclose(f);
    return 0;
}

