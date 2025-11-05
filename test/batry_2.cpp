// I2c_ADS1115_main.cpp
// Programa autónomo com função main que usa o ADS1115 para medir Vbus e Vshunt,
// calculando tensão, corrente e potência. Baseado no ficheiro sem classe que
// te forneci antes. Ajusta R_SHUNT, R_TOP, R_BOT, e endereco I2C conforme o teu hardware.
//
// Compilar:
// g++ -std=c++11 I2c_ADS1115_main.cpp -o I2c_ADS1115_main
//
// Uso:
// sudo ./I2c_ADS1115_main [i2c_device] [i2c_addr]
// Exemplo:
// sudo ./I2c_ADS1115_main /dev/i2c-1 0x48
//
// Nota: executa com permissões adequadas para abrir /dev/i2c-*
// Nota: este exemplo faz leituras single-shot sequenciais (Vshunt depois Vbus).
// Se precisares de leituras contínuas ou performance maior, adapta o código.

#include <cstdint>
#include <string>
#include <stdexcept>
#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <cerrno>
#include <cstring>
#include <cstdlib>

// Registradores ADS1115
#define ADS_REG_CONVERSION  0x00
#define ADS_REG_CONFIG      0x01
#define ADS_REG_LO_THRESH   0x02
#define ADS_REG_HI_THRESH   0x03

// Globals (sem classe)
int     ADS_status = 1;
double  ADS_Voltage = 0.0; // Volt (Vbus)
double  ADS_Current = 0.0; // Ampere
double  ADS_Power   = 0.0; // Watt
uint8_t ADS_addr = 0x48;   // endereço I2C do ADS1115 (ajusta se necessário)
int     ADS_fd = -1;
std::string ADS_i2c_device = "/dev/i2c-1";

// Hardware specifics (ajusta conforme o teu circuito)
const double R_SHUNT = 0.1;     // Ohm (resistor shunt)
const double R_TOP   = 30000.0; // ohm do divisor de tensão para Vbus
const double R_BOT   = 10000.0; // ohm do divisor de tensão para Vbus

// Helpers: escreve 16-bit no registrador do ADS1115
void ADS_writeRegister16(int fd, uint8_t reg, uint16_t value) {
    uint8_t buffer[3];
    buffer[0] = reg;
    buffer[1] = (value >> 8) & 0xFF;
    buffer[2] = value & 0xFF;
    ssize_t w = write(fd, buffer, 3);
    if (w != 3) {
        throw std::runtime_error(std::string("Erro ao escrever reg ADS (write): ") + std::strerror(errno));
    }
}

// Helpers: lê 16-bit do registrador do ADS1115
uint16_t ADS_readRegister16(int fd, uint8_t reg) {
    ssize_t w = write(fd, &reg, 1);
    if (w != 1) {
        throw std::runtime_error(std::string("Erro ao selecionar reg ADS (write): ") + std::strerror(errno));
    }
    uint8_t buffer[2];
    ssize_t r = read(fd, buffer, 2);
    if (r != 2) {
        throw std::runtime_error(std::string("Erro ao ler reg ADS (read): ") + std::strerror(errno));
    }
    return (uint16_t)((buffer[0] << 8) | buffer[1]);
}

// Inicializa o I2C e define o endereço do ADS1115
void ADS_init(uint8_t addr, const std::string &i2c_device) {
    if (ADS_fd >= 0) {
        close(ADS_fd);
        ADS_fd = -1;
    }

    if (ADS_status == 1) {
        ADS_addr = addr;
        ADS_i2c_device = i2c_device;
        ADS_status = 0;
    }

    ADS_fd = open(ADS_i2c_device.c_str(), O_RDWR);
    if (ADS_fd < 0) {
        std::string err = std::string("Erro ao abrir barramento I2C: ") + std::strerror(errno);
        throw std::runtime_error(err);
    }

    if (ioctl(ADS_fd, I2C_SLAVE, ADS_addr) < 0) {
        close(ADS_fd);
        ADS_fd = -1;
        throw std::runtime_error("Failed to set I2C address - ADS1115");
    }
}

// Configura e lê ADS1115 para obter Vshunt (diferencial AIN0-AIN1) e Vbus (single-ended AIN2).
// Actualiza ADS_Voltage, ADS_Current, ADS_Power.
void ADS_update_values() {
    if (ADS_fd < 0) {
        std::cerr << "ADS_update_values: dispositivo I2C não inicializado\n";
        return;
    }

    try {
        // --- Configurar e ler Vshunt (AIN0 - AIN1) ---
        // OS = 1 (start single-shot)
        // MUX = 000 (AIN0 - AIN1 differential)
        // PGA = 110 (±0.512V) para margem; usa 111 (±0.256V) se Vshunt_max < 0.256V
        // MODE = 1 (single-shot)
        // DR = 100 (128SPS)
        // COMP = 11 (comparator disable default)
        uint16_t config_shunt = 0;
        config_shunt |= (1 << 15);           // OS = 1 start single conversion
        config_shunt |= (0b000 << 12);       // MUX = AIN0 - AIN1
        config_shunt |= (0b110 << 9);        // PGA = 0b110 -> ±0.512V
        config_shunt |= (1 << 8);            // MODE = single-shot
        config_shunt |= (0b100 << 5);        // DR = 128 SPS
        config_shunt |= 0x03;                // comparator disable (default)

        ADS_writeRegister16(ADS_fd, ADS_REG_CONFIG, config_shunt);
        usleep(12000); // 12 ms

        uint16_t raw_shunt_u16 = ADS_readRegister16(ADS_fd, ADS_REG_CONVERSION);
        int16_t raw_shunt = (int16_t)raw_shunt_u16; // signed

        // LSB para PGA = ±0.512V -> LSB = 15.625 µV
        double lsb_shunt = 15.625e-6;
        double v_shunt = raw_shunt * lsb_shunt; // V
        double current_A = v_shunt / R_SHUNT; // A

        // --- Configurar e ler Vbus (single-ended em AIN2) ---
        // MUX single-ended AIN2 = 0b110 (ver datasheet)
        // PGA = 001 -> ±4.096V (LSB = 125 µV)
        uint16_t config_bus = 0;
        config_bus |= (1 << 15);              // OS = 1 start
        config_bus |= (0b110 << 12);          // MUX = AIN2 single-ended
        config_bus |= (0b001 << 9);           // PGA = 0b001 -> ±4.096V
        config_bus |= (1 << 8);               // MODE = single-shot
        config_bus |= (0b100 << 5);           // DR = 128 SPS
        config_bus |= 0x03;                   // comparator disable

        ADS_writeRegister16(ADS_fd, ADS_REG_CONFIG, config_bus);
        usleep(12000); // 12 ms

        uint16_t raw_bus_u16 = ADS_readRegister16(ADS_fd, ADS_REG_CONVERSION);
        int16_t raw_bus = (int16_t)raw_bus_u16;

        double lsb_bus = 125e-6;
        double v_adc = raw_bus * lsb_bus; // tensão medida no ADC (após divisor)
        double vbus = v_adc * ((R_TOP + R_BOT) / R_BOT);

        ADS_Voltage = vbus;
        ADS_Current = current_A;
        ADS_Power   = ADS_Voltage * ADS_Current;

        // Debug / info
        std::cout << "Vshunt raw = " << raw_shunt << ", v_shunt = " << v_shunt << " V\n";
        std::cout << "I = " << current_A << " A\n";
        std::cout << "Vbus raw  = " << raw_bus << ", v_adc = " << v_adc << " V, Vbus = " << vbus << " V\n";
        std::cout << "Power = " << ADS_Power << " W\n";
    }
    catch (const std::exception &e) {
        std::cerr << "Failed update Values ADS: " << e.what() << "\n";
        return;
    }
}

// Fecha file descriptor I2C
void ADS_close() {
    if (ADS_fd >= 0) {
        close(ADS_fd);
        ADS_fd = -1;
    }
}

int main(int argc, char *argv[]) {
    // Parâmetros opcionais: i2c_device e addr
    std::string dev = "/dev/i2c-1";
    uint8_t addr = 0x41;

    if (argc >= 2) dev = argv[1];
    if (argc >= 3) {
        // aceita "0x48" ou decimal "72"
        addr = static_cast<uint8_t>(std::strtoul(argv[2], nullptr, 0));
    }

    try {
        std::cout << "Inicializando ADS1115 em " << dev << " addr 0x" << std::hex << (int)addr << std::dec << "\n";
        ADS_init(addr, dev);

        // Faz várias leituras com intervalo
        for (int i = 0; i < 5; ++i) {
            std::cout << "Leitura " << (i+1) << "...\n";
            ADS_update_values();
            // Impressão resumida
            std::cout << "Resumo: V = " << ADS_Voltage << " V, I = " << ADS_Current << " A, P = " << ADS_Power << " W\n\n";
            sleep(1);
        }

        ADS_close();
    } catch (const std::exception &e) {
        std::cerr << "Erro no main: " << e.what() << "\n";
        ADS_close();
        return 1;
    }

    return 0;
}
