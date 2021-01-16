#include "ADXL345.h"
#include "main.h"

extern I2C_HandleTypeDef hi2c2;

Vector r_new;
Vector n_new;
Vector f_new;
Activites a_new;
adxl345_range_t _range_new;

bool ADXL345_begin(void) {
  f_new.XAxis = 0;
  f_new.YAxis = 0;
  f_new.ZAxis = 0;
  // Check ADXL345 REG DEVID
  if (ADXL345_fastRegister8(ADXL345_REG_DEVID) != 0xE5) {
      return false;
  }
  // Enable measurement mode (0b00001000)
	//ADXL345_writeRegister8(ADXL345_REG_POWER_CTL, 0x00);
  ADXL345_writeRegister8(ADXL345_REG_POWER_CTL, 0x08);
  ADXL345_clearSettings();
  return true;
}

// Set Range
void ADXL345_setRange(adxl345_range_t range) {
  // Get actual value register
  uint8_t value = ADXL345_readRegister8(ADXL345_REG_DATA_FORMAT);
  // Update the data rate
  // (&) 0b11110000 (0xF0 - Leave HSB)
  // (|) 0b0000xx?? (range - Set range)
  // (|) 0b00001000 (0x08 - Set Full Res)
  value &= 0xF0;
  value |= range;
  value |= 0x08;
  ADXL345_writeRegister8(ADXL345_REG_DATA_FORMAT, value);
}

// Get Range
adxl345_range_t ADXL345_getRange(void) {
	return (adxl345_range_t)(ADXL345_readRegister8(ADXL345_REG_DATA_FORMAT) & 0x03);
}

// Set Data Rate
void ADXL345_setDataRate(adxl345_dataRate_t dataRate) {
	ADXL345_writeRegister8(ADXL345_REG_BW_RATE, dataRate);
}

// Get Data Rate
adxl345_dataRate_t ADXL345_getDataRate(void) {
	return (adxl345_dataRate_t)(ADXL345_readRegister8(ADXL345_REG_BW_RATE) & 0x0F);
}

// Low Pass Filter
Vector ADXL345_lowPassFilter(Vector vector, double alpha) {
	f_new.XAxis = vector.XAxis * alpha + (f_new.XAxis * (1.0 - alpha));
	f_new.YAxis = vector.YAxis * alpha + (f_new.YAxis * (1.0 - alpha));
	f_new.ZAxis = vector.ZAxis * alpha + (f_new.ZAxis * (1.0 - alpha));
	return f_new;
}

// Read raw values
Vector ADXL345_readRaw(void) {
	r_new.XAxis = (double)ADXL345_readRegister16(ADXL345_REG_DATAX0);
	r_new.YAxis = (double)ADXL345_readRegister16(ADXL345_REG_DATAY0); 
	r_new.ZAxis = (double)ADXL345_readRegister16(ADXL345_REG_DATAZ0); 
	return r_new;
}

// Read normalized values
Vector ADXL345_readNormalize(double gravityFactor) {
	ADXL345_readRaw();
	// (4 mg/LSB scale factor in Full Res) * gravity factor
	n_new.XAxis = r_new.XAxis * 0.004 * gravityFactor;
	n_new.YAxis = r_new.YAxis * 0.004 * gravityFactor;
	n_new.ZAxis = r_new.ZAxis * 0.004 * gravityFactor;
	return n_new;
}

// Read scaled values
Vector ADXL345_readScaled(void) {
	ADXL345_readRaw();
	// (4 mg/LSB scale factor in Full Res)
	n_new.XAxis = r_new.XAxis * 0.004;
	n_new.YAxis = r_new.YAxis * 0.004;
	n_new.ZAxis = r_new.ZAxis * 0.004;
	return n_new;
}

void ADXL345_clearSettings(void) {
	ADXL345_setRange(ADXL345_RANGE_2G);
	ADXL345_setDataRate(ADXL345_DATARATE_100HZ);
	
	ADXL345_writeRegister8(ADXL345_REG_THRESH_TAP, 0x00);
	ADXL345_writeRegister8(ADXL345_REG_DUR, 0x00);
	ADXL345_writeRegister8(ADXL345_REG_LATENT, 0x00);
	ADXL345_writeRegister8(ADXL345_REG_WINDOW, 0x00);
	ADXL345_writeRegister8(ADXL345_REG_THRESH_ACT, 0x00);
	ADXL345_writeRegister8(ADXL345_REG_THRESH_INACT, 0x00);
	ADXL345_writeRegister8(ADXL345_REG_TIME_INACT, 0x00);
	ADXL345_writeRegister8(ADXL345_REG_THRESH_FF, 0x00);
	ADXL345_writeRegister8(ADXL345_REG_TIME_FF, 0x00);
	
	uint8_t value;
	value = ADXL345_readRegister8(ADXL345_REG_ACT_INACT_CTL);
	value &= 0x88;
	ADXL345_writeRegister8(ADXL345_REG_ACT_INACT_CTL, value);
	
	value = ADXL345_readRegister8(ADXL345_REG_TAP_AXES);
	value &= 0xF8;
	ADXL345_writeRegister8(ADXL345_REG_TAP_AXES, value);
}

// Set Tap Threshold (62.5mg / LSB)
void ADXL345_setTapThreshold(double threshold) {
	uint8_t scaled = constrain(threshold / 0.0625f, 0, 255);
  ADXL345_writeRegister8(ADXL345_REG_THRESH_TAP, scaled);
}

// Get Tap Threshold (62.5mg / LSB)
double ADXL345_getTapThreshold(void) {
  return ADXL345_readRegister8(ADXL345_REG_THRESH_TAP) * 0.0625f;
}

// Set Tap Duration (625us / LSB)
void ADXL345_setTapDuration(double duration) {
  uint8_t scaled = constrain(duration / 0.000625f, 0, 255);
  ADXL345_writeRegister8(ADXL345_REG_DUR, scaled);
}

// Get Tap Duration (625us / LSB)
double ADXL345_getTapDuration(void) {
  return ADXL345_readRegister8(ADXL345_REG_DUR) * 0.000625f;
}

// Set Double Tap Latency (1.25ms / LSB)
void ADXL345_setDoubleTapLatency(double latency) {
  uint8_t scaled = constrain(latency / 0.00125f, 0, 255);
  ADXL345_writeRegister8(ADXL345_REG_LATENT, scaled);
}

// Get Double Tap Latency (1.25ms / LSB)
double ADXL345_getDoubleTapLatency() {
  return ADXL345_readRegister8(ADXL345_REG_LATENT) * 0.00125f;
}

// Set Double Tap Window (1.25ms / LSB)
void ADXL345_setDoubleTapWindow(double window) {
  uint8_t scaled = constrain(window / 0.00125f, 0, 255);
  ADXL345_writeRegister8(ADXL345_REG_WINDOW, scaled);
}

// Get Double Tap Window (1.25ms / LSB)
double ADXL345_getDoubleTapWindow(void) {
  return ADXL345_readRegister8(ADXL345_REG_WINDOW) * 0.00125f;
}

// Set Activity Threshold (62.5mg / LSB)
void ADXL345_setActivityThreshold(double threshold) {
  uint8_t scaled = constrain(threshold / 0.0625f, 0, 255);
  ADXL345_writeRegister8(ADXL345_REG_THRESH_ACT, scaled);
}

// Get Activity Threshold (65.5mg / LSB)
double ADXL345_getActivityThreshold(void) {
  return ADXL345_readRegister8(ADXL345_REG_THRESH_ACT) * 0.0625f;
}

// Set Inactivity Threshold (65.5mg / LSB)
void ADXL345_setInactivityThreshold(double threshold) {
  uint8_t scaled = constrain(threshold / 0.0625f, 0, 255);
  ADXL345_writeRegister8(ADXL345_REG_THRESH_INACT, scaled);
}

// Get Incactivity Threshold (65.5mg / LSB)
double ADXL345_getInactivityThreshold(void) {
  return ADXL345_readRegister8(ADXL345_REG_THRESH_INACT) * 0.0625f;
}

// Set Inactivity Time (s / LSB)
void ADXL345_setTimeInactivity(uint8_t time) {
  ADXL345_writeRegister8(ADXL345_REG_TIME_INACT, time);
}

// Get Inactivity Time (s / LSB)
uint8_t ADXL345_getTimeInactivity(void) {
  return ADXL345_readRegister8(ADXL345_REG_TIME_INACT);
}

// Set Free Fall Threshold (65.5mg / LSB)
void ADXL345_setFreeFallThreshold(double threshold) {
  uint8_t scaled = constrain(threshold / 0.0625f, 0, 255);
	ADXL345_writeRegister8(ADXL345_REG_THRESH_FF, scaled);
}

// Get Free Fall Threshold (65.5mg / LSB)
double ADXL345_getFreeFallThreshold(void) {
  return ADXL345_readRegister8(ADXL345_REG_THRESH_FF) * 0.0625f;
}

// Set Free Fall Duratiom (5ms / LSB)
void ADXL345_setFreeFallDuration(double duration) {
  uint8_t scaled = constrain(duration / 0.005f, 0, 255);
  ADXL345_writeRegister8(ADXL345_REG_TIME_FF, scaled);
}

// Get Free Fall Duratiom
double ADXL345_getFreeFallDuration(void) {
  return ADXL345_readRegister8(ADXL345_REG_TIME_FF) * 0.005f;
}

void ADXL345_setActivityX(uint8_t state) {
  ADXL345_writeRegisterBit(ADXL345_REG_ACT_INACT_CTL, 6, state);
}

bool ADXL345_getActivityX(void) {
  return ADXL345_readRegisterBit(ADXL345_REG_ACT_INACT_CTL, 6);
}

void ADXL345_setActivityY(uint8_t state) {
  ADXL345_writeRegisterBit(ADXL345_REG_ACT_INACT_CTL, 5, state);
}

bool ADXL345_getActivityY(void) {
  return ADXL345_readRegisterBit(ADXL345_REG_ACT_INACT_CTL, 5);
}

void ADXL345_setActivityZ(uint8_t state) {
  ADXL345_writeRegisterBit(ADXL345_REG_ACT_INACT_CTL, 4, state);
}

bool ADXL345_getActivityZ(void) {
  return ADXL345_readRegisterBit(ADXL345_REG_ACT_INACT_CTL, 4);
}

void ADXL345_setActivityXYZ(uint8_t state) {
  uint8_t value;
  value = ADXL345_readRegister8(ADXL345_REG_ACT_INACT_CTL);
  if (state) {
		value |= 0X38;
  } 
	else {
		value &= 0XC7;
  }
  ADXL345_writeRegister8(ADXL345_REG_ACT_INACT_CTL, value);
}

void ADXL345_setInactivityX(uint8_t state) {
  ADXL345_writeRegisterBit(ADXL345_REG_ACT_INACT_CTL, 2, state);
}

bool ADXL345_getInactivityX(void) {
  return ADXL345_readRegisterBit(ADXL345_REG_ACT_INACT_CTL, 2);
}

void ADXL345_setInactivityY(uint8_t state) {
  ADXL345_writeRegisterBit(ADXL345_REG_ACT_INACT_CTL, 1, state);
}

bool ADXL345_getInactivityY(void) {
  return ADXL345_readRegisterBit(ADXL345_REG_ACT_INACT_CTL, 1);
}

void ADXL345_setInactivityZ(uint8_t state) {
  ADXL345_writeRegisterBit(ADXL345_REG_ACT_INACT_CTL, 0, state);
}

bool ADXL345_getInactivityZ(void) {
  return ADXL345_readRegisterBit(ADXL345_REG_ACT_INACT_CTL, 0);
}

void ADXL345_setInactivityXYZ(uint8_t state) {
  uint8_t value;
  value = ADXL345_readRegister8(ADXL345_REG_ACT_INACT_CTL);
  if (state) {
		value |= 0X07;
  } 
	else {
		value &= 0xf8;
  }
  ADXL345_writeRegister8(ADXL345_REG_ACT_INACT_CTL, value);
}

void ADXL345_setTapDetectionX(uint8_t state) {
  ADXL345_writeRegisterBit(ADXL345_REG_TAP_AXES, 2, state);
}

bool ADXL345_getTapDetectionX(void) {
  return ADXL345_readRegisterBit(ADXL345_REG_TAP_AXES, 2);
}

void ADXL345_setTapDetectionY(uint8_t state) {
  ADXL345_writeRegisterBit(ADXL345_REG_TAP_AXES, 1, state);
}

bool ADXL345_getTapDetectionY(void) {
  return ADXL345_readRegisterBit(ADXL345_REG_TAP_AXES, 1);
}

void ADXL345_setTapDetectionZ(uint8_t state) {
  ADXL345_writeRegisterBit(ADXL345_REG_TAP_AXES, 0, state);
}

bool ADXL345_getTapDetectionZ(void) {
  return ADXL345_readRegisterBit(ADXL345_REG_TAP_AXES, 0);
}

void ADXL345_setTapDetectionXYZ(uint8_t state) {
  uint8_t value;
  value = ADXL345_readRegister8(ADXL345_REG_TAP_AXES);
  if (state) {
		value |= 0X07;
  } 
	else {
		value &= 0XF8;
  }
  ADXL345_writeRegister8(ADXL345_REG_TAP_AXES, value);
}


void ADXL345_useInterrupt(adxl345_int_t interrupt) {
  if (interrupt == 0) {
		ADXL345_writeRegister8(ADXL345_REG_INT_MAP, 0x00);
  } 
	else {
		ADXL345_writeRegister8(ADXL345_REG_INT_MAP, 0xFF);
  }
  ADXL345_writeRegister8(ADXL345_REG_INT_ENABLE, 0xFF);
}

Activites ADXL345_readActivites(void) {
  uint8_t data = ADXL345_readRegister8(ADXL345_REG_INT_SOURCE);

  a_new.isOverrun = ((data >> ADXL345_OVERRUN) & 1);
  a_new.isWatermark = ((data >> ADXL345_WATERMARK) & 1);
  a_new.isFreeFall = ((data >> ADXL345_FREE_FALL) & 1);
  a_new.isInactivity = ((data >> ADXL345_INACTIVITY) & 1);
  a_new.isActivity = ((data >> ADXL345_ACTIVITY) & 1);
  a_new.isDoubleTap = ((data >> ADXL345_DOUBLE_TAP) & 1);
  a_new.isTap = ((data >> ADXL345_SINGLE_TAP) & 1);
  a_new.isDataReady = ((data >> ADXL345_DATA_READY) & 1);

  data = ADXL345_readRegister8(ADXL345_REG_ACT_TAP_STATUS);

  a_new.isActivityOnX = ((data >> 6) & 1);
  a_new.isActivityOnY = ((data >> 5) & 1);
  a_new.isActivityOnZ = ((data >> 4) & 1);
  a_new.isTapOnX = ((data >> 2) & 1);
  a_new.isTapOnY = ((data >> 1) & 1);
  a_new.isTapOnZ = ((data >> 0) & 1);

  return a_new;
}

// Write byte to register
void ADXL345_writeRegister8(uint8_t reg, uint8_t value) {
	uint8_t DATA[2] = { reg, value };
	HAL_I2C_Master_Transmit(&hi2c2, ADXL345_ADDRESS_WRITE, DATA, 2, 10);
}

// Read byte to register
uint8_t ADXL345_fastRegister8(uint8_t reg) {
  uint8_t value;
	HAL_I2C_Master_Transmit(&hi2c2, ADXL345_ADDRESS_WRITE, &reg, 1, 10);
  HAL_I2C_Master_Receive(&hi2c2, ADXL345_ADDRESS_READ, &value, 1, 10);  
  return value;
}

// Read byte from register
uint8_t ADXL345_readRegister8(uint8_t reg) {
  uint8_t value;
	HAL_I2C_Master_Transmit(&hi2c2, ADXL345_ADDRESS_WRITE, &reg, 1, 10);
  HAL_I2C_Master_Receive(&hi2c2, ADXL345_ADDRESS_READ, &value, 1, 10);  
  return value;
}

// Read word from register
int16_t ADXL345_readRegister16(uint8_t reg) {
	int16_t value;
  uint8_t value_data[2];
	HAL_I2C_Master_Transmit(&hi2c2, ADXL345_ADDRESS_WRITE, &reg, 1, 10);
  HAL_I2C_Master_Receive(&hi2c2, ADXL345_ADDRESS_READ, value_data, 2, 10);  
  value = (value_data[1] << 8) | value_data[0];
  return value;
}

void ADXL345_writeRegisterBit(uint8_t reg, uint8_t pos, uint8_t state) {
  uint8_t value;
  value = ADXL345_readRegister8(reg);
  if (state) {
		value |= (1 << pos);
  } 
	else {
		value &= ~(1 << pos);
  }
  ADXL345_writeRegister8(reg, value);
}

bool ADXL345_readRegisterBit(uint8_t reg, uint8_t pos) {
  uint8_t value;
  value = ADXL345_readRegister8(reg);
  return ((value >> pos) & 1);
}
