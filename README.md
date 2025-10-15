# TEAM1_I2C



<h5>What the i2c ?</h5>

<p>
The Inter-Integrated Circuit (I²C) protocol is a widely used communication standard
in electronics for connecting multiple low-speed peripherals to a microcontroller or
processor using just two wires: SDA (data line) and SCL (clock line).</p>

Como usar:

Para usarmos o protocol i2c no raspbery temos te ter dois valores identificados a paltaforma i2c do
raspberry que estamos a usar como a "/dev/i2c-1" e dentro dela vamos ter uma tablea de enderecos que 
podemos aceder usado o command " i2cdetect -y 1 "

veremos algo do genero:

'''
    0 1 2 3 4 5 6 7 8 9 a b c d e f
00:          -- -- -- -- -- -- -- -- 
10: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
20: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
30: -- -- -- -- -- -- -- -- 38 -- -- -- -- -- -- -- 
40: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 

'''

No i2c temos 2 tipos de controlador o main que contrala todos os outros modulos i2c 
sendo possivle main de um controlador main nao sendo muito usado.


Em c podemos usar a biblioteca  #include <linux/i2c-dev.h> para comunicsao de i2c.

'''
open() → abrir barramento

ioctl() → configurar endereço ou parâmetros especiais

read()/write() → trocar dados

'''

A fusao prinspla e a ioctl(fd, funsionalidade, endereco) esta fusao permite setar mods no i2c sendo eles:

'''
I2C_SLAVE	Seleciona o endereço do escravo (7 ou 10 bits)
I2C_SLAVE_FORCE	Força comunicação mesmo se o barramento estiver ocupado
I2C_TENBIT	Define que o escravo usa endereço de 10 bits
I2C_FUNCS	Pergunta ao driver quais funcionalidades ele suporta
I2C_RDWR	Realiza uma operação de leitura/escrita combinada (i2c_rdwr_ioctl_data)
'''

PCA9685 -
 E um controlador que gera sitnais de PWM(Pulse Width Modulation) podendo ser usado em servos e motores dc;
 Usando o protocol de comunicsao i2c para comunicsao com microcontrollers.



