#ifndef __PCA9555_H
#define __PCA9555_H

#include "type.h"
#include "board.h"

//---Chip selection---
#define PCA9555_BASE_ADDR 0x20

//---Command byte---
#define INPUT_PORT0          0x00
#define INPUT_PORT1          0x01
#define OUTPUT_PORT0         0x02
#define OUTPUT_PORT1         0x03
#define POL_INV0             0x04
#define POL_INV1             0x05
#define CONF_PORT0           0x06
#define CONF_PORT1           0x07

//---I2C configuration---
#define SPEED_100KHZ         100000
#define SPEED_400KHZ         400000
#define I2C_DEFAULT_SPEED    SPEED_100KHZ
#define I2C_FASTPLUS_BIT     0

#define DEFAULT_BUF_LEN      4 //2 bytes in hex representation.

//---I2C variables---
static int mode_poll;	/* Poll/Interrupt mode flag */

//---PCA Variables---
uint8_t PCA9555_ADDR;
uint16_t IO_CONFIG;
uint16_t POL_CONFIG;
uint16_t OUTPUT_CONFIG;
uint16_t LAST_INPUT;
uint8_t MSG[DEFAULT_BUF_LEN];
uint8_t RCV[DEFAULT_BUF_LEN];
I2C_ID_T i2cid;

//----- Setup -----

//Sets the address to the given value. Only uses 3 bits (LSB).
void setAddress(uint8_t addr);

static void Init_I2C_PinMux(void);

/* Set I2C mode to polling/interrupt */
static void i2c_set_mode(I2C_ID_T id, int polling);

/* State machine handler for I2C0 and I2C1 */
static void i2c_state_handling(I2C_ID_T id);

/**
 * @brief	I2C Interrupt Handler
 * @return	None
 */
void I2C_IRQHandler(void);

//Initializes connection with the
void initPca9555(I2C_ID_T id, int speed, uint8_t addr);

//----- User functions -----

//Returns I/O configuration.
uint16_t getIo();

//Configures pins for I/O.
int setIo(uint16_t pinModes);



//Returns polarity configuration.
uint16_t getPolarity();

//Sets polarity.
int setPolarity(uint16_t invPins);



//Reads the current output configuration.
uint16_t getOutput();

//Sets output configuration.
int setOutput(uint16_t outputConfig);



//Reads the input.
uint16_t getInput();

//----- Auxiliary -----

//Sets all (len) values in the buffer to 0.
void clearBuffer(uint8_t *buf);

//Copies the src byte array to the dst byte array.
void bytecpy(uint8_t *dst, uint8_t *src, int len);

//----- Communication -----

//Sends dataLength bytes to the given registers (MSB first).
int sendBytes(uint8_t command, uint8_t *data, int dataLength);

//Sends 2 bytes contained in data to the given register(s).
int sendWord(uint8_t reg, uint16_t data);

//Returns an array of bytes from all requested registers.
uint8_t* readBytes(uint8_t command, int len);

//Reads 2 bytes from the given register(s).
uint16_t readWord(uint8_t reg);

#endif
