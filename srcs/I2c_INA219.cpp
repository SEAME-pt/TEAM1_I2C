#include "../include/I2c_INA219.hpp"
#include <cstdint>
#include <iostream>

#define REG_CONFIG             0x00
#define REG_SHUNT_VOLTAGE      0x01
#define REG_BUS_VOLTAGE        0x02
#define REG_POWER              0x03
#define REG_CURRENT            0x04
#define REG_CALIBRATION        0x05

 int 		I2c_INA219::status = 1;
 double  	I2c_INA219::_Voltage; // Volt 
 double 	I2c_INA219::_Current; // Ampere
 double 	I2c_INA219::_Power;    // watt
 uint8_t 	I2c_INA219::_addr;
 int 		I2c_INA219::fd;
 std::string  	I2c_INA219::_i2c_device;

void I2c_INA219::writeRegister(int fd, uint8_t reg, uint16_t value) {
    uint8_t buffer[3];
    buffer[0] = reg;
    buffer[1] = (value >> 8) & 0xFF;
    buffer[2] = value & 0xFF;
    if (write(fd, buffer, 3) != 3) {
	throw std::runtime_error("Erro ao escrever no registrador");

    }
}

uint16_t I2c_INA219::readRegister(int fd, uint8_t reg) {
    if (write(fd, &reg, 1) != 1) {

	throw std::runtime_error("Erro ao selecionar registrador");

    }
    uint8_t buffer[2];
    if (read(fd, buffer, 2) != 2) {

	throw std::runtime_error("Erro ao ler registrador");
    }
    return (buffer[0] << 8) | buffer[1];
}



void I2c_INA219::init( uint8_t addr,std::string i2c_device)
{

if (fd >= 0) close(fd);

   if(status == 1)
    {
	_addr = addr;
	_i2c_device = i2c_device;
	status = 0;
    }
    fd = open(_i2c_device.c_str(), O_RDWR);
    if (fd < 0) {
        perror("Erro ao abrir barramento I2C");
        close(fd);
        throw std::runtime_error( "Erro ao abrir barramento I2C");
    }

    if (ioctl(fd, I2C_SLAVE, _addr) < 0) {
        close(fd);
        throw std::runtime_error("Failed to set I2C address-INA219");
    }

	 uint16_t config = 0x19FF;
    writeRegister(fd, REG_CONFIG, config);

    uint16_t calibration = 4096;;
    writeRegister(fd, REG_CALIBRATION, calibration);

usleep(10000);
}



void I2c_INA219::update_values()
{
    try
    {
        // ===== Leitura =====
        uint16_t shunt_raw = readRegister(fd, REG_SHUNT_VOLTAGE);
        uint16_t bus_raw = readRegister(fd, REG_BUS_VOLTAGE);
        uint16_t current_raw = readRegister(fd, REG_CURRENT);
        uint16_t power_raw = readRegister(fd, REG_POWER);
        
        std::cout << "Bus raw = 0x" << std::hex << bus_raw << std::dec << std::endl;
        
        // Shunt voltage: LSB = 10µV
        double shunt_voltage = (int16_t)shunt_raw * 0.00001; // V (10 µV/bit)
        
        // Bus voltage: deslocar 3 bits e aplicar máscara de 13 bits
        // LSB = 4mV = 0.004V
        double bus_voltage = ((bus_raw >> 3) & 0x1FFF) * 0.0045; // V
        
        std::cout << "Bus voltage = " << bus_voltage << " V, Shunt voltage = " << shunt_voltage << " V" << std::endl;
        
        _Voltage = bus_voltage; // tensão total (VIN+)
        _Current = (int16_t)current_raw * 0.0978;  // mA (depende da calibração)
        _Power   = power_raw * 1.956;              // mW (depende da calibração)
    }
    catch(std::exception &e)
    {
        std::cout << "Failed to update values: " << e.what() << std::endl;
        return;
    }
}

void I2c_INA219::print()
{
	I2c_INA219::update_values();

    std::cout << "==========================" << std::endl;
    std::cout << "INA219 - finich write" << std::endl;
    std::cout << "--------------------------" << std::endl;
    std::cout << "Voltage (Vbus): " << _Voltage << " V" << std::endl;
    std::cout << "Current: " << _Current << " mA" << std::endl;
    std::cout << "Power: " << _Power << " mW" << std::endl;
    std::cout << "==========================" << std::endl;


}

void I2c_INA219::close_()
{
	close(fd);
}


