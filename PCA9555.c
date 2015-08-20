#include "type.h"
#include "PCA9555.h"

//----- Setup -----

void setAddress(uint8_t addr){
	PCA9555_ADDR = PCA9555_BASE_ADDR | (addr & 0x07);
}

static void Init_I2C_PinMux(void)
{
#if (defined(BOARD_NXP_XPRESSO_11U14) || defined(BOARD_NGX_BLUEBOARD_11U24))
	Chip_SYSCTL_PeriphReset(RESET_I2C0);
	Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 4, IOCON_FUNC1 | I2C_FASTPLUS_BIT);
	Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 5, IOCON_FUNC1 | I2C_FASTPLUS_BIT);
#elif (defined(BOARD_NXP_XPRESSO_11C24) || defined(BOARD_MCORE48_1125))
	Chip_SYSCTL_PeriphReset(RESET_I2C0);
	Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO0_4, IOCON_FUNC1 | I2C_FASTPLUS_BIT);
	Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO0_5, IOCON_FUNC1 | I2C_FASTPLUS_BIT);
#else
	#error "No Pin Muxing defined for I2C operation"
#endif
}

static void i2c_set_mode(I2C_ID_T id, int polling)
{
	if (!polling) {
		mode_poll &= ~(1 << id);
		Chip_I2C_SetMasterEventHandler(id, Chip_I2C_EventHandler);
		NVIC_EnableIRQ(I2C0_IRQn);
	}
	else {
		mode_poll |= 1 << id;
		NVIC_DisableIRQ(I2C0_IRQn);
		Chip_I2C_SetMasterEventHandler(id, Chip_I2C_EventHandlerPolling);
	}
}

/* State machine handler for I2C0 and I2C1 */
static void i2c_state_handling(I2C_ID_T id)
{
	if (Chip_I2C_IsMasterActive(id)) {
		Chip_I2C_MasterStateHandler(id);
	}
	else {
		Chip_I2C_SlaveStateHandler(id);
	}
}

/**
 * @brief	I2C Interrupt Handler
 * @return	None
 */
void I2C_IRQHandler(void)
{
	i2c_state_handling(I2C0);
}

void initPca9555(I2C_ID_T id, int speed, uint8_t addr){
	Init_I2C_PinMux();

	setAddress(addr);
	i2cid = id;
	Chip_I2C_Init(id);
	Chip_I2C_SetClockRate(id, speed);

	/* Set default mode to interrupt */
	i2c_set_mode(id, 0);
}

//----- User functions -----

uint16_t getIo(){
	//return IO_CONFIG;
	return readWord(CONF_PORT0);
}

int setIo(uint16_t pinModes){
	IO_CONFIG = pinModes;
	return sendWord(CONF_PORT0, pinModes);
}

uint16_t getPolarity(){
	//return POL_CONFIG;
	return readWord(POL_INV0);
}

int setPolarity(uint16_t invPins){
	POL_CONFIG = invPins;
	return sendWord(POL_INV0, invPins);
}

uint16_t getOutput(){
	//return OUTPUT_CONFIG;
	return readWord(OUTPUT_PORT0);
}

int setOutput(uint16_t outputConfig){
	OUTPUT_CONFIG = outputConfig;
	return sendWord(OUTPUT_PORT0, outputConfig);
}

uint16_t getInput(){
	LAST_INPUT = readWord(INPUT_PORT0);
	return LAST_INPUT;
}

//----- Auxiliary -----
void clearBuffer(uint8_t *buf){
	for(int i = 0; i < DEFAULT_BUF_LEN; i++){
		buf[i] = 0;
	}
}

void bytecpy(uint8_t *dst, uint8_t *src, int len){
	for(int i = 0; i < len; i++){
		dst[i] = src[i];
	}
}

//----- Communication -----
int sendBytes(uint8_t command, uint8_t *data, int dataLength){
	clearBuffer(MSG);
	MSG[0] = command;
	bytecpy(MSG+1, data, dataLength);
	return Chip_I2C_MasterSend(i2cid, PCA9555_ADDR, MSG, dataLength + 1);
}

int sendWord(uint8_t reg, uint16_t data){
	uint8_t buf[2] = {(uint8_t)(data>>8), (uint8_t)data};
	return sendBytes(reg, buf, 2);
}

uint8_t* readBytes(uint8_t command, int len){
	if(len <= 0) len = 2;
	clearBuffer(RCV);
	Chip_I2C_MasterCmdRead(i2cid, PCA9555_ADDR, command, RCV, len);
	return RCV;
}

uint16_t readWord(uint8_t reg){
	uint8_t *config8 = readBytes(reg, 2);
	uint16_t config16 = config8[1]|(config8[0]<<8);
	return config16;
}
