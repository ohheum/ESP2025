/******************************************************************************
i2ctest.cpp
Raspberry Pi I2C interface demo
Byron Jacquot @ SparkFun Electronics>
4/2/2014
https://github.com/sparkfun/Pi_Wedge

A brief demonstration of the Raspberry Pi I2C interface, using the SparkFun
Pi Wedge breakout board.

Resources:

This example makes use of the Wiring Pi library, which streamlines the interface
the the I/O pins on the Raspberry Pi, providing an API that is similar to the
Arduino.  You can learn about installing Wiring Pi here:
http://wiringpi.com/download-and-install/

The I2C API is documented here:
https://projects.drogon.net/raspberry-pi/wiringpi/i2c-library/

The init call returns a standard file descriptor.  More detailed configuration
of the interface can be performed using ioctl calls on that descriptor.
See the wiringPi I2C implementation (wiringPi/wiringPiI2C.c) for some examples.
Parameters configurable with ioctl are documented here:
http://git.kernel.org/cgit/linux/kernel/git/torvalds/linux.git/tree/Documentation/i2c/dev-interface

Hardware connections:

This file interfaces with the SparkFun MCP4725 breakout board:
https://www.sparkfun.com/products/8736

The board was connected as follows:
(Raspberry Pi)(MCP4725)
GND  -> GND
3.3V -> Vcc
SCL  -> SCL
SDA  -> SDA

An oscilloscope probe was connected to the analog output pin of the MCP4725.

To build this file, I use the command:
>  g++ i2ctest.cpp -lwiringPi

Then to run it, first the I2C kernel module needs to be loaded.  This can be 
done using the GPIO utility.
> gpio load i2c 400
> ./a.out

This will run the MCP through its output range several times.  A rising 
sawtooth will be seen on the analog output.

Development environment specifics:
Tested on Raspberry Pi V2 hardware, running Raspbian.
Building with GCC 4.6.3 (Debian 4.6.3-14+rpi1)

This code is beerware; if you see me (or any other SparkFun employee) at the
local, and you've found our code helpful, please buy us a round!

Distributed as-is; no warranty is given.
******************************************************************************/

#include <iostream>
#include <errno.h>
#include <wiringPiI2C.h>
#include<unistd.h>

using namespace std;

int main()
{
   int fd = wiringPiI2CSetup(0x68);
   if (fd < 0) {
      cout << "Falied to open I2C device. " << endl;
      return 1;
   }

   int result = wiringPiI2CWriteReg8(fd, 0x6B, 0x80);
   if(result == -1)
   {
      cout << "Failed to enable temperature sensor." << endl;
      return 1;
   }

   unsigned char temp[6];
   for(int i = 0; i < 10; i++)
   {
      temp[0] = wiringPiI2CReadReg8(fd, 0x3B);
      temp[1] = wiringPiI2CReadReg8(fd, 0x3C);
      temp[2] = wiringPiI2CReadReg8(fd, 0x3D);
      temp[3] = wiringPiI2CReadReg8(fd, 0x3E);
      temp[4] = wiringPiI2CReadReg8(fd, 0x3F);
      temp[5] = wiringPiI2CReadReg8(fd, 0x40);

	   float AccX = (int16_t)(temp[0] << 8 | temp[1]) / 16384.0;
	   float AccY = (int16_t)(temp[2] << 8 | temp[3]) / 16384.0;
	   float AccZ = (int16_t)(temp[4] << 8 | temp[5]) / 16384.0;

      cout << "AccX/AccY/AccZ: " << AccX << "/" << AccY << "/" << AccZ << endl;
      sleep(1);
   }
}
