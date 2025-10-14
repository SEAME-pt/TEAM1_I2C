
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <iostream>


class I2c
{
	private:
		int addr;
		
	public:
		I2c(int addr);
};
