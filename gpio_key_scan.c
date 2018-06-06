#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/select.h>
#include <sys/time.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <linux/input.h>
#include "connect.h"

#define KEY_PRESSED (0)
#define KEY_RELEASED (1)
#define KEY_CODE (2)

#define DEV_KEY "/dev/input/event2"
int key_event_monitor(bool *run_flag, int keys_fd)
{
    if (!run_flag) {
        fprintf(stderr, "Called %s failed, param error\n", __func__);
        return -1;
    }
    
    struct input_event t;

    while (*run_flag) {
        fd_set fdset;
		FD_ZERO(&fdset);
		FD_SET(keys_fd, &fdset);
		struct timeval valid_time = {0, 10 * 1000};

		int ret = -1;
		ret = select(keys_fd + 1, &fdset, NULL, NULL, &valid_time);
		if (ret > 0) {
			if (FD_ISSET(keys_fd, &fdset)) {
				if (read(keys_fd, &t, sizeof(t)) == sizeof(t)) {
					if (EV_KEY == t.type && KEY_CODE == t.code) {
                        if (KEY_PRESSED == t.value) {
                            KEY_INPUT = 1;
                        }
                        else if (KEY_RELEASED == t.value) {
                            KEY_INPUT = 0;
                        }
                    }
                }
            }
        }
        else if (ret < 0) {
            fprintf(stderr, "Called select failed, errno:%d, %s.\n", errno, strerror(errno));
        }        
    }
}

int key_open(const char *dev_file)
{
    if (!dev_file) {
        fprintf(stderr, "Called %s failed, param error.\n", __func__);
        return -1;
    }

    int keys_fd = open(dev_file, O_RDONLY);
	if (keys_fd <= 0) {
        fprintf(stderr, "[%s : %d]open %s device error!\n", __FILE__, __LINE__, dev_file);
	    return -1;
	}

    return keys_fd;
}

int key_close(int *keys_fd)
{
    if (*keys_fd > 0) {
        close(*keys_fd);
    }

    return 0;
}

void* key_scan_thread(void *arg)
{
    int fd = -1;
    bool run_flag = true;
    
    assert((fd = key_open(DEV_KEY)) > 0);
    key_event_monitor(&run_flag,fd);
    key_close(&fd);
}