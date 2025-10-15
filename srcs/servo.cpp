#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <cstdint>
#include <stdexcept>
#include <cmath>

class PCA9685 {
public:
    PCA9685(const char* i2c_device, uint8_t address) : addr(address) {
        if ((file = open(i2c_device, O_RDWR)) < 0) {
            throw std::runtime_error("Failed to open I2C device");
        }
        if (ioctl(file, I2C_SLAVE, addr) < 0) {
            throw std::runtime_error("Failed to set I2C address");
        }
    }

    ~PCA9685() {
        close(file);
    }

    void write_byte(uint8_t reg, uint8_t val) {
        uint8_t buffer[2] = {reg, val};
        if (write(file, buffer, 2) != 2) {
            throw std::runtime_error("Failed to write I2C byte");
        }
    }

    void set_pwm(uint8_t channel, uint16_t on, uint16_t off) {
        uint8_t reg_base = 0x06 + 4 * channel;
        write_byte(reg_base, on & 0xFF);
        write_byte(reg_base + 1, on >> 8);
        write_byte(reg_base + 2, off & 0xFF);
        write_byte(reg_base + 3, off >> 8);
    }

    void init(uint8_t prescaler) {
        write_byte(0x00, 0x00); // MODE1 normal
        usleep(5000);
        write_byte(0x01, 0x04); // MODE2 totem pole
        usleep(5000);
        write_byte(0x00, 0x10); // MODE1 sleep
        usleep(5000);
        write_byte(0xFE, prescaler); // Set prescaler
        usleep(5000);
        write_byte(0x00, 0x80); // Exit sleep
        usleep(5000);
    }

    // ===========================
    // Funções para servo motor
    // ===========================

    // Converte ângulo 0°-180° para valor PWM 0-4095
    uint16_t angle_to_pwm(float angle) {
        if (angle < 0.0f) angle = 0.0f;
        if (angle > 180.0f) angle = 180.0f;

        const float min_pulse_ms = 1.0f;  // 0°
        const float max_pulse_ms = 2.0f;  // 180°
        const float freq = 50.0f;         // Hz
        const float period_ms = 1000.0f / freq; // ~20ms

        float pulse_ms = min_pulse_ms + (angle / 180.0f) * (max_pulse_ms - min_pulse_ms);
        uint16_t pwm_val = static_cast<uint16_t>((pulse_ms / period_ms) * 4095);
        return pwm_val;
    }

    void set_servo_angle(uint8_t channel, float angle) {
        uint16_t pwm = angle_to_pwm(angle);
        set_pwm(channel, 0, pwm);
    }

    void stop_all() {
        for (uint8_t ch = 0; ch < 16; ++ch) {
            set_pwm(ch, 0, 0);
        }
    }

private:
    int file;
    uint8_t addr;
};

// ===========================
// Exemplo de uso
// ===========================
int main() {
    try {
        PCA9685 pca("/dev/i2c-1", 0x40);
        pca.init(121); // Prescaler para ~50Hz

        // Teste de servo
        std::cout << "Servo 0 para 0°\n";
        pca.set_servo_angle(0, 0);
        sleep(1);

        std::cout << "Servo 0 para 90°\n";
        pca.set_servo_angle(0, 90);
        sleep(1);

        std::cout << "Servo 0 para 180°\n";
        pca.set_servo_angle(0, 180);
        sleep(1);

        pca.stop_all();

    } catch (const std::exception &e) {
        std::cerr << "Erro: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}

