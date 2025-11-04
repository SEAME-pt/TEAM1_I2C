#include "../include/I2c.hpp"

int main()
{

	std::cout << "inicialaze the i2c dispositive" << std::endl;
	I2c::All_init();
	
//	I2c::motor(0, 50, 1);
	I2c::print();
	sleep(1);
	I2c::print();

	I2c::All_close();	
	
}
