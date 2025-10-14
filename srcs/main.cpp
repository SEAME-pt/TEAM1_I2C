#include <iostream>
#include <fcntl.h>      // open()
#include <unistd.h>     // read(), write(), close()
#include <sys/ioctl.h>  // ioctl()
#include <linux/i2c-dev.h> // interface I2C
#include <cstdint>      // uint8_t, etc.
#include <string>

#define I2C_DEVICE "/dev/i2c-1" // Barramento padrão no Raspberry Pi

// ============================================================================
// Classe base: I2CDevice
// ============================================================================
class I2CDevice {
protected:
    int fd;              // File descriptor do dispositivo
    int address;         // Endereço I2C
    std::string device;  // Caminho do barramento I2C

public:
    I2CDevice(const std::string& dev, int addr)
        : fd(-1), address(addr), device(dev) {}

    virtual ~I2CDevice() {
        if (fd >= 0) close(fd);
    }

    // Abre o dispositivo e configura o endereço
    bool openDevice() {
        fd = open(device.c_str(), O_RDWR);
        if (fd < 0) {
            perror("Erro ao abrir dispositivo I2C");
            return false;
        }
        if (ioctl(fd, I2C_SLAVE, address) < 0) {
            perror("Erro ao configurar endereço I2C");
            close(fd);
            fd = -1;
            return false;
        }
        return true;
    }

    // Escreve um byte em um registrador
    bool writeReg8(uint8_t reg, uint8_t value) {
        uint8_t buffer[2] = {reg, value};
        if (write(fd, buffer, 2) != 2) {
            perror("Erro ao escrever no barramento I2C");
            return false;
        }
        return true;
    }

    // Escreve múltiplos bytes
    bool writeBytes(const uint8_t* data, size_t length) {
        if (write(fd, data, length) != (ssize_t)length) {
            perror("Erro ao enviar bytes via I2C");
            return false;
        }
        return true;
    }
};

// ============================================================================
// Classe derivada: Motor (usa PCA9685 como controlador PWM)
// ============================================================================
class Motor : public I2CDevice {
private:
    int channel; // Canal do motor (0–15)

public:
    Motor(const std::string& dev, int addr, int ch)
        : I2CDevice(dev, addr), channel(ch) {}

    // Configura o driver PCA9685
    bool setup() {
        if (!openDevice()) return false;
        std::cout << "Configurando PCA9685 no endereço 0x" 
                  << std::hex << address << "..." << std::endl;
        return writeReg8(0x00, 0x00); // MODO1: normal
    }

    // Define o PWM (posição on/off de 12 bits)
    void setPWM(int on, int off) {
        uint8_t reg = 0x06 + 4 * channel;
        uint8_t data[5] = {
            reg,
            static_cast<uint8_t>(on & 0xFF),
            static_cast<uint8_t>(on >> 8),
            static_cast<uint8_t>(off & 0xFF),
            static_cast<uint8_t>(off >> 8)
        };
        if (!writeBytes(data, 5)) {
            std::cerr << "Falha ao ajustar PWM para o canal " << channel << std::endl;
        }
    }

    // Liga o motor (duty cycle entre 0 e 4095)
    void ligar(int duty = 2048) {
        std::cout << "Ligando motor no canal " << channel
                  << " com duty " << duty << std::endl;
        setPWM(0, duty);
    }

    // Desliga o motor
    void desligar() {
        std::cout << "Desligando motor no canal " << channel << std::endl;
        setPWM(0, 0);
    }
};

// ============================================================================
// Programa principal
// ============================================================================
int main() {
    Motor motor(I2C_DEVICE, 0x40, 0); // endereço 0x40, canal 0

    if (!motor.setup()) {
        std::cerr << "Falha na inicialização do motor!" << std::endl;
        return 1;
    }

    motor.ligar(2048); // 50% de velocidade
    sleep(5);          // mantém o motor ligado
    motor.desligar();

    std::cout << "Finalizado." << std::endl;
    return 0;
}

