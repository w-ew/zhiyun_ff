#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termio.h>
#include <sys/ioctl.h>
#include <asm/ioctls.h>


#define poly 0x1021
int crc16(char *addr, int num, int crc)
{
int i;

for (; num>0; num--)               /* Step through bytes in memory */
  {
  crc = crc ^ (*addr++ << 8);      /* Fetch byte from memory, XOR into CRC top byte*/
  for (i=0; i<8; i++)              /* Prepare to rotate 8 bits */
    {
    crc = crc << 1;                /* rotate */
    if (crc & 0x10000)             /* bit 15 was set (now bit 16)... */
      crc = (crc ^ poly) & 0xFFFF; /* XOR with XMODEM polynomic */
                                   /* and ensure CRC remains 16-bit value */
    }                              /* Loop for 8 bits */
  }                                /* Loop until num=0 */
  return(crc);                     /* Return updated CRC */
}


int main(int argc, char *argv[]) {

	volatile uint8_t msg[] = { 0xA5, 0x5A, 0x0A, 0xFF, 0x0F, 0x00, 0x08, 0x00, 0x08, /* header */
		0x80, 0x00, /* pos */
		0x00, 0x00, /* unknown */
		0x00, 0x00 /* crc */
	};

	struct termios tty;
	int pos;
	uint16_t crc;

	pos = 0;
	if(argc >= 2) {
		pos = atoi(argv[1]);
	};

	//msg[9] = pos >> 8;
	msg[10] = pos;

	//crc = crc_ccitt(0, msg, sizeof(msg)-2);
	crc = crc16(msg, 13, 0);
	msg[13] = crc;
	msg[14] = crc >> 8;

	for(int i=0; i<sizeof(msg); i++) printf("%02X ", msg[i]);
	printf("\n");

	int file = open("/dev/ttyUSB0", O_RDWR);
	tcgetattr(file, &tty);

	tty.c_cflag &= ~( PARENB | PARODD| CSTOPB | CSIZE | CRTSCTS);
	tty.c_cflag |=  ( /*CMSPAR |*/ CS8 | CREAD | CLOCAL);
	
	

	tty.c_lflag &= ~(ICANON | ECHO | ECHOE | ECHONL | ISIG);

	tty.c_iflag &= ~(IXON | IXOFF | IXANY);
	tty.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL);

	tty.c_oflag &= ~(OPOST | ONLCR);

	tty.c_cc[VTIME] = 10; /* 1s */
	tty.c_cc[VMIN] = 0;

	cfsetospeed(&tty, B115200);
	cfsetispeed(&tty, B115200);

	tcsetattr(file, TCSANOW, &tty);

	int mod = 0;

	while(1) {
		mod += 1;
		if(mod == 1) {
			mod = 0;
			pos += 2;
		};
		//pos += 1;
		//msg[9] = pos >> 8;
		msg[10] = pos;
		//crc = crc_ccitt(0, msg, sizeof(msg)-2);
		crc = crc16(msg, sizeof(msg)-2, 0);
		msg[13] = crc;
		msg[14] = crc >> 8;

		write(file, msg, sizeof(msg));
		for(int i=0; i<sizeof(msg); i++) printf("%02X ", msg[i]);
		printf("\n");

		usleep(5000);

	};

	close(file);

	return 0;
};
