
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <iostream>



class I2c
{
	protected:
		static int _fd_mot;
		static int _fd_servo;
		static int _fd_set;
		static std::string  _i2c_device;
		static void write_byte(uint8_t reg, uint8_t val);
		static void set_pwm(uint8_t channel, uint16_t on, uint16_t off);
		static void set_pwm_duty(uint8_t channel, float duty_fraction);

	public:
		static void init(uint8_t addr_mot, uint8_t addr_servo,std::string i2c_device);
		static void stop_all();
		static void stop_motors();
		static void motor(int mot,int speed,int dir);
};
