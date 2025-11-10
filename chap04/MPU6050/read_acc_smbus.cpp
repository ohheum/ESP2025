#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <iostream>
#include <math.h>

extern "C" {
    #include <linux/i2c.h>
    #include <linux/i2c-dev.h>
    #include <i2c/smbus.h>
}
using namespace std;

#define MPU6050  			0x68
#define ACCEL_OUT			0x3B
#define PWR_MGMT_1			0x6B

#define MAX_BUS 64
#define FAILED 1
#define SUCCESS 0

float AccX, AccY, AccZ;
unsigned char xmAddress;
int I2CBus;

int file;

int initI2C();
unsigned char I2CreadByte(unsigned char address, unsigned char subAddress);
unsigned char xmReadByte(unsigned char subAddress);
void readAcc();
int begin(int bus, unsigned char xmAddr);
void xmReadBytes(unsigned char subAddress, unsigned char * dest, unsigned char count);
void I2CreadBytes(unsigned char address, unsigned char subAddress, unsigned char * dest, unsigned char count);
int initMPU6050();
int I2CwriteByte(unsigned char address, unsigned char subAddress, unsigned char data);
int xmWriteByte(unsigned char subAddress, unsigned char data);
	
int main() {
	int status = begin(1, MPU6050);    /* 1 for /dev/i2c-1, 2 for /dev/i2c-2 */
	if (status) {
	   	cout << "Starting I2C and Sensor failed." << endl;
		return 1;
	}
	sleep(1);
	while (1) {
		readAcc();
		cout << "AccX/AccY/AccZ: " << AccX << "/" << AccY << "/" << AccZ << endl;
		sleep(1);
	}
}

int begin(int bus, unsigned char xmAddr)
{
	I2CBus = bus;  		
	xmAddress = xmAddr;
	if (initI2C()) 	{	// Initialize I2C
		cout << "I2C initi failed" << endl;
		return FAILED;
	}					
	if (initMPU6050()) {
		cout << "Reset MPU6050 failed" << endl;
		return FAILED;
	}
	return SUCCESS;
}

int initI2C()
{
	//Open I2C file as bidirectional
	char namebuf[MAX_BUS];
	snprintf(namebuf, sizeof(namebuf), "/dev/i2c-%d", I2CBus);
	if ((file=open(namebuf, O_RDWR)) < 0) {
		cout << "Failed to open sensor on " << namebuf << " I2C Bus" << endl;
		return FAILED;
	}
	return SUCCESS;
}

int initMPU6050()
{	
	/* CTRL_REG5_XM enables temp sensor, sets mag resolution and data rate
	Bits (7-0): TEMP_EN M_RES1 M_RES0 M_ODR2 M_ODR1 M_ODR0 LIR2 LIR1
	TEMP_EN - Enable temperature sensor (0=disabled, 1=enabled)	 */
	return xmWriteByte(PWR_MGMT_1, 0x80); // enable temperature sensor
}

void readAcc()
{
	unsigned char temp[6]; // We'll read two chars from the temperature sensor into temp	
	xmReadBytes(ACCEL_OUT, temp, 6); // Read 2 chars, beginning at OUT_TEMP_L_M

    // for (int i=0; i<6; i++) {
    //     printf("%x ", temp[i]);
    // }
	// printf("\n");

	AccX = (int16_t)(temp[0] << 8 | temp[1]) / 16384.0;
	AccY = (int16_t)(temp[2] << 8 | temp[3]) / 16384.0;
	AccZ = (int16_t)(temp[4] << 8 | temp[5]) / 16384.0;
}

unsigned char I2CreadByte(unsigned char address, unsigned char subAddress)
{
	if (ioctl(file, I2C_SLAVE, address) < 0) {
		cout << "I2C_SLAVE address " << address << " failed..." << endl;
	}
	return i2c_smbus_read_byte_data(file, subAddress);
}

unsigned char xmReadByte(unsigned char subAddress)
{
	// Whether we're using I2C or SPI, read a char using the
	// accelerometer-specific I2C address or SPI CS pin.
	return I2CreadByte(xmAddress, subAddress);
}

void I2CreadBytes(unsigned char address, unsigned char subAddress, unsigned char * dest, unsigned char count)
{  
	if (ioctl(file, I2C_SLAVE, address) < 0) {
		cout << "I2C_SLAVE address " << address << " failed..." << endl;
	}
	// unsigned char sAddr = subAddress | 0x80; // or with 0x80 to indicate multi-read
	int result = i2c_smbus_read_i2c_block_data(file, subAddress, count, dest);
	if (result != count) {
		printf("Failed to read block from I2C");
	}
}

void xmReadBytes(unsigned char subAddress, unsigned char * dest, unsigned char count)
{
	// Whether we're using I2C or SPI, read multiple chars using the
	// accelerometer-specific I2C address or SPI CS pin.
	I2CreadBytes(xmAddress, subAddress, dest, count);
}

int xmWriteByte(unsigned char subAddress, unsigned char data)
{
	// Whether we're using I2C or SPI, write a char using the
	// accelerometer-specific I2C address or SPI CS pin.
	return I2CwriteByte(xmAddress, subAddress, data);
}

int I2CwriteByte(unsigned char address, unsigned char subAddress, unsigned char data)
{
	if (ioctl(file, I2C_SLAVE, address) < 0) {
		cout << "I2C_SLAVE address " << address << " failed..." << endl;
		return FAILED;
	}
	int result = i2c_smbus_write_byte_data(file, subAddress, data);
	if (result == -1) {
		printf("Failed to write byte to " + subAddress);
		return FAILED;
	}
	return SUCCESS;
}




