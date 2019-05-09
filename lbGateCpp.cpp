#include <stdint.h>
#include <errno.h>
#include <fcntl.h> 
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>

#define WRITE_PERIOD_IN_US 1000000

int main()
{
	char portname[] = "/dev/ext";
	int fd = -1;
	char write_buffer[] = "ext ping get\n";
	uint8_t read_buffer[50] = {0};
	char rxMsgRef[] = "ext ping get\n";
	uint32_t loopNb = 0;

	fd = open(portname, O_RDWR | O_NOCTTY | O_NDELAY | O_NONBLOCK);
	if (fd < 0) {
		printf("ERROR opening %s\n", portname);
		return -1;
	}

	struct termios oldtty0, newtty0;

	tcgetattr(fd, &oldtty0);
	newtty0 = oldtty0;

	newtty0.c_cflag = B115200 | CS8 | CREAD | CLOCAL;
	newtty0.c_iflag = 0;
	newtty0.c_oflag = 0;
	newtty0.c_lflag = 0;
	newtty0.c_cc[VMIN] = 0;
	newtty0.c_cc[VTIME] = 0;

	// Flush terminal I/O buffers and activate new settings immediately.
	tcflush(fd, TCIFLUSH);
	tcsetattr(fd, TCSANOW, &newtty0);

	// WRITE PERIODICALLY
	while (1) {
		int write_count = write(fd, write_buffer, strnlen(write_buffer, 100));
		if (write_count !=  strnlen(write_buffer, 100)) {
			printf("ERROR writing: wrong nb of bytes at %d\n", loopNb);
		}

		usleep(WRITE_PERIOD_IN_US);

		int read_count = read(fd, read_buffer, sizeof(read_buffer));
		if(strnlen(rxMsgRef, 100) == read_count) {
			if(0 == strncmp(read_buffer, rxMsgRef, 100)) {
				printf("OK rx at %d\n", loopNb);
			}
			else {
				printf("ERROR msg at %d: %s\n", loopNb, read_buffer);
			}
		}
		else {
			printf("ERROR Rx Msg Len at %d %s\n", loopNb, read_buffer);
		}
	}
	
}

