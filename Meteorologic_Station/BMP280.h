
#ifndef INC_BMP280_STM32_H_
#define INC_BMP280_STM32_H_

#include "main.h"

#define BMP280_I2C hi2c1

#define SUPPORT_64BIT 1
//#define SUPPORT_32BIT 1

// OLHAR O PINO FISICO SDO
#define BMP280_ADDRESS 0xEC  // SDO is grounded, the 7 bit address is 0x76 and 8 bit address = 0x76<<1 = 0xEC

extern float Temperature, Pressure;

extern I2C_HandleTypeDef BMP280_I2C;


int BMP280_Config (uint8_t osrs_t, uint8_t osrs_p, uint8_t mode, uint8_t t_sb, uint8_t filter);

/* To be used when doing the force measurement
 * the Device need to be put in forced mode every time the measurement is needed
 */
void BMP280_WakeUP(void);

/* measure the temp, pressure and humidity
 * the values will be stored in the parameters passed to the function
 */
void BMP280_Measure(float *temperature, float *pressure);


// Oversampling definitions
#define OSRS_OFF    	0x00
#define OSRS_1      	0x01
#define OSRS_2      	0x02
#define OSRS_4      	0x03
#define OSRS_8      	0x04
#define OSRS_16     	0x05

// MODE Definitions
#define MODE_SLEEP      0x00
#define MODE_FORCED     0x01
#define MODE_NORMAL     0x03

// Standby Time
#define T_SB_0p5    	0x00
#define T_SB_62p5   	0x01
#define T_SB_125    	0x02
#define T_SB_250    	0x03
#define T_SB_500    	0x04
#define T_SB_1000   	0x05
#define T_SB_10     	0x06
#define T_SB_20     	0x07

// IIR Filter Coefficients
#define IIR_OFF     	0x00
#define IIR_2       	0x01
#define IIR_4       	0x02
#define IIR_8       	0x03
#define IIR_16      	0x04


// REGISTERS DEFINITIONS
#define ID_REG      	0xD0
#define RESET_REG  		0xE0
#define STATUS_REG      0xF3
#define CTRL_MEAS_REG   0xF4
#define CONFIG_REG      0xF5
#define PRESS_MSB_REG   0xF7


#endif /* INC_BMP280_STM32_H_ */
