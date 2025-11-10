#include <stdio.h> 
#include <stdlib.h>
#include <fcntl.h> 
#include <sys/ioctl.h> 
#include <linux/i2c.h> 
#include <linux/i2c-dev.h> 
#include <unistd.h>

int writeRegister(int file, unsigned int registerAddress, unsigned char value){
   unsigned char buffer[2];
   buffer[0] = registerAddress;
   buffer[1] = value;
   if(write(file, buffer, 2)!=2){
      perror("I2C: Failed write to the device\n");
      return 1;
   }
   return 0;
}

int main(){
    int file;
    float AccX, AccY, AccZ;

    printf("Starting the MPU6050 test application\n"); 
    if((file=open("/dev/i2c-1", O_RDWR)) < 0){
        perror("failed to open the bus\n");
        return 1; 
    }
    if(ioctl(file, I2C_SLAVE, 0x68) < 0){
      perror("Failed to connect to the sensor\n");
      return 1;
    }

    /* Read 0x1C register(ACCEL_CONFIG) just for test */
    char writeBuffer[1] = {0x1C}; 
    if(write(file, writeBuffer, 1)!=1){
        perror("Failed to reset the read address\n");
        return 1; 
    }

    char buf[1];
    if(read(file, buf, 1)!=1){
        perror("Failed to read in the buffer\n");
        return 1; 
    }
    printf("ACCEL_CONFIG at 0x1C: %x\n", buf[0]);

    /* Reset device by writing 0x80 into PWR_MGMT_1 register*/
    writeRegister(file, 0x6B, 0x80);

    /* read acc data */
    unsigned char acc[6];
    char accReg[] = {0x3B, 0x03C, 0x3D, 0x3E, 0x3F, 0x40}; 
    for(int i=0; i<10; i++) {
        for (int i=0; i<6; i++) {
            if(write(file, (accReg+i), 1)!=1){
                perror("Failed to reset the read address\n");
                return 1; 
            }   
            if(read(file, (acc+i), 1)!=1){
                perror("Failed to read in the buffer\n");
                return 1; 
            }
        }

        AccX = (int16_t)(acc[0] << 8 | acc[1]) / 16384.0;
        AccY = (int16_t)(acc[2] << 8 | acc[3]) / 16384.0;
        AccZ = (int16_t)(acc[4] << 8 | acc[5]) / 16384.0;

        printf("AccX/AccY/AccZ: %f %f %f\n", AccX, AccY, AccZ);
    }
    close(file);
    return 0;
}
