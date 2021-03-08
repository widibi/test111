#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <fcntl.h>
#include <time.h>
#include <sys/ioctl.h>
#include <linux/ioctl.h>
#include <sys/stat.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>


static const char *device = "/dev/spidev1.0";
static int iterations;
#define BUFFERS_SIZE 32
static uint8_t bits = 8;
static uint32_t speed = 100000;

static void pabort(const char *s)
{
	perror(s);
	abort();
}


int setConfig(int fd)
{
	int ret=0;
	/*
	 * bits per word
	 */
	ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
	if (ret == -1)
		pabort("can't set bits per word");

	ret = ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits);
	if (ret == -1)
		pabort("can't get bits per word");

	/*
	 * max speed hz
	 */
	ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
	if (ret == -1)
		pabort("can't set max speed hz");

	ret = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
	if (ret == -1)
		pabort("can't get max speed hz");

	return ret;
}


int setConfig01(int fd)
{
	int ret=0;
	/*
	 * bits per word
	 */
	ret = ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits);
	if (ret == -1)
		pabort("can't get bits per word");

	/*
	 * max speed hz
	 */
	ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
	if (ret == -1)
		pabort("can't set max speed hz");

	ret = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
	if (ret == -1)
		pabort("can't get max speed hz");

	printf("bits per word: %d\n", bits);
	printf("max speed: %d Hz (%d KHz)\n", speed, speed/1000);

	return ret;
}

int readConfig01(int fd)
{
	int ret=0;
	/*
	 * bits per word
	 */
	ret = ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits);
	if (ret == -1)
		pabort("can't get bits per word");

	/*
	 * max speed hz
	 */
	ret = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
	if (ret == -1)
		pabort("can't get max speed hz");

	printf("bits per word: %d\n", bits);
	printf("max speed: %d Hz (%d KHz)\n", speed, speed/1000);

	return ret;
}



int read_func(int fd,int len)
{
	char *rx,*bp;
	int ret=0;

	rx = malloc(len);

	memset(rx,0,len);

	setConfig01(fd);
	ret=read(fd,rx,len);
	if (ret == -1)
		pabort("can't read");

	printf("response(%2d, %2d): ", len, ret);
	for (bp = rx; len; len--)
		printf(" %02x", *bp++);
	printf("\n");

	free(rx);

	return ret;

}

int read_funcEx(int fd,int len)
{
	char *tx;
	char *rx,*bp;;
	int ret=0;

	tx = malloc(len);
	rx = malloc(len);

	memset(rx,0,len);
	memset(tx,0,len);

	struct spi_ioc_transfer tr = {
		.tx_buf = (unsigned long)tx,
		.rx_buf = (unsigned long)rx,
		.len = len,
		.delay_usecs = 1000,
		.speed_hz = speed,
		.bits_per_word = bits,
	};
		
	//readConfig01(fd);
	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
	if (ret == -1)
		pabort("can't transfer");
	else
	{
		printf("response(%2d, %2d): ", len, ret);
		for (bp = rx; len; len--)
		printf(" %02x", *bp++);
		printf("\n");
	}

	free(tx);
	free(rx);

}


static void read_func02(int fd, int len)
{
	struct spi_ioc_transfer	xfer[2];
	unsigned char		buf[32], *bp;
	int			status;

	memset(xfer, 0, sizeof xfer);
	memset(buf, 0, sizeof buf);

	if (len > sizeof buf)
		len = sizeof buf;

	buf[0] = 0xaa;
	xfer[0].tx_buf = (unsigned long)buf;
	xfer[0].len = 1;

	xfer[1].rx_buf = (unsigned long) buf;
	xfer[1].len = len;

	setConfig01(fd);
	status = ioctl(fd, SPI_IOC_MESSAGE(2), xfer);
	if (status < 0) {
		pabort("SPI_IOC_MESSAGE");
		return;
	}

	printf("response(%2d, %2d): ", len, status);
	for (bp = buf; len; len--)
		printf(" %02x", *bp++);
	printf("\n");
}



int main(int argc, char *argv[])
{
	int ret = 0;
	int fd;

	
	iterations=100000;

	fd = open(device, O_RDWR);
	if (fd < 0)
		pabort("can't open device");

	ret=setConfig(fd);
	if(ret==-1)
	{
		pabort("set configuration failed!!!");
		return ret;
	}

	while(1)
	{
		if(iterations>0)
		{	
			read_func(fd,BUFFERS_SIZE);
			
			iterations--;
		}
		else
		{
			break;
		}
	}


	close(fd);

	return ret;
}




