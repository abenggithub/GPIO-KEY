#include <stdio.h>
#include <pthread.h>

#include "gpio_key_scan.h"
#include "gpio_key_analyze.h"

int main(int argc, char *argv[])
{
    pthread_t scan_tid;
    pthread_t analyze_tid;

    int ret = pthread_create(&analyze_tid, NULL, key_analyze_thread, NULL);
    if (ret != 0) {
        fprintf(stderr, "called pthread_create failed, error num:%d.\n", ret);
        return -1;
    }

    ret = pthread_create(&scan_tid, NULL, key_scan_thread, NULL);
    if (ret != 0) {
        fprintf(stderr, "called pthread_create failed, error num:%d.\n", ret);
        return -1;
    }
    
    pthread_join(scan_tid, NULL);
    pthread_join(analyze_tid, NULL);
    return 0;
}