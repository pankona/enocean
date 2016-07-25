/* serialread.c */

#include <stdio.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>

#define SERIAL_PORT "/dev/tty.usbserial-FT5YBYZ"
#define BUFLEN (1024)

static int OK = 0;
static int ERROR = -1;

static struct termios oldtio, newtio;
static int fd = -1;
static pthread_mutex_t mutex;

static void *
readSerial(void *in_arg)
{
	int i, ret;
    int count = 0;
	unsigned char buf[BUFLEN] = {0};

	// variables for timeout of select()
	fd_set set;
	struct timeval timeout;
	FD_ZERO(&set);
	FD_SET(fd, &set);

	while (1) {
		memset(buf, 0, BUFLEN);

        ret = select(fd + 1, &set, NULL, NULL, NULL);
        if (ret == -1) {
            // error by select
            printf("select error\n");
            return 0; 
        }

        ret = read(fd, buf, BUFLEN-1);
        if (ret < 0) {
            printf("[%s][%d] read error ret = %d\n", __func__, __LINE__, ret);
            continue;
        }

        for (i = 0; i < ret; i++) {
            printf("%02x ", buf[i]);
            count++;
            if (count % 16 == 0) {
                printf("\n");
            }
        }
	}
	return 0;
}

static void
restoreSerialPortConfiguration()
{
	// restore backed up serial port configuration
	tcsetattr(fd, TCSANOW, &oldtio);
	close(fd);
}

int
enOceanOpen()
{
	int ret;
	pthread_t pthread;

	fd = open(SERIAL_PORT, O_RDWR | O_NOCTTY | O_NONBLOCK);
	if (fd < 0) {
		printf("failed to open serial port\n");
		return ERROR;
	}

	ret = tcgetattr(fd, &oldtio);
	if (ret != 0) {
		printf("tcgetattr failed. ret = %d", ret);
		return ERROR;
	}
	newtio = oldtio;
	ret = cfsetspeed(&newtio, B57600);
	if (ret != 0) {
		printf("cfsetspeed failed. ret = %d", ret);
		return ERROR;
	}

    ret = tcflush(fd, TCIFLUSH);
	if (ret != 0) {
		printf("tcflush failed. ret = %d", ret);
		return ERROR;
	}
    ret = tcsetattr(fd, TCSANOW, &newtio);
	if (ret != 0) {
		printf("tcsetattr failed. ret = %d", ret);
		return ERROR;
	}

	ret = pthread_create(&pthread, NULL, readSerial, NULL);
	if (ret != 0) {
		// fatal error
		restoreSerialPortConfiguration();
		return ERROR;
	}

	pthread_mutex_init(&mutex, NULL);

	return OK;
}

int
enOceanClose()
{
	restoreSerialPortConfiguration();
	return OK;
}

