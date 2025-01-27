/*!
 * @file     Adafruit_MMA8451.h
 *
 * @mainpage Adafruit MMA8451 Accelerometer Library
 *
 * @section intro_sec Introduction
 *
 * This is a library for the Adafruit MMA8451 Accel breakout board
 * ----> https://www.adafruit.com/products/2019
 *
 * Adafruit invests time and resources providing this open source code,
 * please support Adafruit and open-source hardware by purchasing
 * products from Adafruit!
 *
 * @author   K. Townsend (Adafruit Industries)
 *
 * @section  HISTORY
 *
 * v1.0  - First release
 *
 * @ section license License
 *
 * BSD (see license.txt)
 */

#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include <Adafruit_MMA8451.h>
#include <Wire.h>

#include <SEGGER_RTT.h>

#define rwrite_string SEGGER_RTT_WriteString
#define rprintf SEGGER_RTT_printf

/**************************************************************************/
/*!
    @brief  Abstract away platform differences in Arduino wire library
*/
/**************************************************************************/
static inline uint8_t i2cread(void) {
#if ARDUINO >= 100
  return Wire.read();
#else
  return Wire.receive();
#endif
}

static inline void i2cwrite(uint8_t x) {
#if ARDUINO >= 100
  Wire.write((uint8_t) x);
#else
  Wire.send(x);
#endif
}

/**************************************************************************/
/*!
    @brief  Writes 8-bits to the specified destination register
*/
/**************************************************************************/
void Adafruit_MMA8451::writeRegister8(uint8_t reg, uint8_t value) {
  Wire.beginTransmission(_i2caddr);
  i2cwrite((uint8_t) reg);
  i2cwrite((uint8_t) (value));
  Wire.endTransmission();
}

/**************************************************************************/
/*!
    @brief  Reads 8-bits from the specified register
*/
/**************************************************************************/
uint8_t Adafruit_MMA8451::readRegister8(uint8_t reg) {
// undocumented version of requestFrom handles repeated starts on Arduino Due
#ifdef __SAM3X8E__
  Wire.requestFrom(_i2caddr, 1, reg, 1, true);
#else
  // I don't know - maybe the other verion of requestFrom works on all
  // platforms.
  //  honestly, I don't want to go through and test them all.  Doing it this way
  //  is already known to work on everything else
  Wire.beginTransmission(_i2caddr);
  i2cwrite(reg);
  Wire.endTransmission(false);  // MMA8451 + friends uses repeated start!!
  Wire.requestFrom(_i2caddr, 1);
#endif

  if (!Wire.available()) return -1;
  return (i2cread());
}

/**************************************************************************/
/*!
    @brief  Instantiates a new MMA8451 class in I2C mode
*/
/**************************************************************************/
Adafruit_MMA8451::Adafruit_MMA8451(int32_t sensorID) { _sensorID = sensorID; }

/**************************************************************************/
/*!
    @brief  Setups the HW (reads coefficients values, etc.)
*/
/**************************************************************************/
bool Adafruit_MMA8451::begin(uint8_t i2caddr, const transient_cfg_t *transient_cfg) {
  Wire.begin();
  _i2caddr = i2caddr;

  /* Check connection */
  uint8_t deviceid = readRegister8(MMA8451_REG_WHOAMI);
  if (deviceid != 0x1A) {
    /* No MMA8451 detected ... return false */
    // Serial.println(deviceid, HEX);
    return false;
  }

  writeRegister8(MMA8451_REG_CTRL_REG2, 0x40);  // reset

  while (readRegister8(MMA8451_REG_CTRL_REG2) & 0x40);

  writeRegister8(MMA8451_REG_XYZ_DATA_CFG, 0 << 4 |  // high pass filter
      MMA8451_RANGE_2_G);
  writeRegister8(MMA8451_REG_HP_FILTER_CUTOFF, 0b00);

  if (transient_cfg != nullptr) {
    writeRegister8(MMA8451_REG_TRANSIENT_CFG, transient_cfg->z_enabled << 3 |
        transient_cfg->y_enabled << 2 |
        transient_cfg->x_enabled << 1);
    writeRegister8(MMA8451_REG_TRANSIENT_THS, transient_cfg->threshold & ~(1 << 7));
    writeRegister8(MMA8451_REG_TRANSIENT_THS, 10);
    writeRegister8(MMA8451_REG_TRANSIENT_COUNT, transient_cfg->debounce_count);
  }

  writeRegister8(MMA8451_REG_CTRL_REG2,
      //MMA8451_SLPE_ENABLE |         // enable auto-sleep mode
                 MMA8451_LOW_POWER >> 3u |  // sleep mode oversampling
                     MMA8451_LOW_POWER);       // active mode oversampling

  // DRDY on INT1
  writeRegister8(MMA8451_REG_CTRL_REG4, 0b00100001);  // TRANS and DRDY
  writeRegister8(MMA8451_REG_CTRL_REG5, 0x01);        // DRDY int 1, TRANS int 2

  // Turn on orientation config
  // writeRegister8(MMA8451_REG_PL_CFG, 0x40);

  writeRegister8(MMA8451_REG_CTRL_REG1,
                 (MMA8451_SLEEP_DATARATE_12_5_HZ << 6u) |
                     (MMA8451_DATARATE_12_5_HZ << 3u) |
                     0b101);  // low noise, fast read, activate

  /*
  for (uint8_t i=0; i<0x30; i++) {
    Serial.print("$");
    Serial.print(i, HEX); Serial.print(" = 0x");
    Serial.println(readRegister8(i), HEX);
  }
  */

  return true;
}

void Adafruit_MMA8451::setTransientConfiguration(const transient_cfg_t *cfg) {
  uint8_t reg1 = readRegister8(MMA8451_REG_CTRL_REG1);
  writeRegister8(MMA8451_REG_CTRL_REG1, 0x00);  // deactivate

  writeRegister8(MMA8451_REG_TRANSIENT_CFG,
                 cfg->z_enabled << 3 |
                     cfg->y_enabled << 2 |
                     cfg->x_enabled << 1);
  writeRegister8(MMA8451_REG_TRANSIENT_THS, cfg->threshold & ~(1 << 7));
  writeRegister8(MMA8451_REG_TRANSIENT_COUNT, cfg->debounce_count);

  writeRegister8(MMA8451_REG_CTRL_REG1, reg1 | 0x01);  // activate
}

void Adafruit_MMA8451::setTransientEventInterrupt(uint8_t num) {
  uint8_t reg1 = readRegister8(MMA8451_REG_CTRL_REG1);
  uint8_t reg4 = readRegister8(MMA8451_REG_CTRL_REG4);
  uint8_t reg5 = readRegister8(MMA8451_REG_CTRL_REG5);

  writeRegister8(MMA8451_REG_CTRL_REG1, 0x00);  // deactivate

  if (num == 1) {
    writeRegister8(MMA8451_REG_CTRL_REG4, reg4 | 1 << 5); // enable interrupt
    writeRegister8(MMA8451_REG_CTRL_REG5, reg5 | 1 << 5); // set to int1
  } else if (num == 2) {
    writeRegister8(MMA8451_REG_CTRL_REG4, reg4 | 1 << 5);
    writeRegister8(MMA8451_REG_CTRL_REG5, reg5 & ~(1 << 5)); // set to int2
  } else {
    writeRegister8(MMA8451_REG_CTRL_REG4, reg4 & ~(1 << 5)); // disable interrupt
  }

  writeRegister8(MMA8451_REG_CTRL_REG1, reg1 | 0x01);  // activate
}

void Adafruit_MMA8451::read(void) {
  // read x y z at once
  Wire.beginTransmission(_i2caddr);
  i2cwrite(MMA8451_REG_OUT_X_MSB);
  Wire.endTransmission(false);  // MMA8451 + friends uses repeated start!!

  Wire.requestFrom(_i2caddr, 6);
  x = Wire.read();
  x <<= 8;
  x |= Wire.read();
  x >>= 2;
  y = Wire.read();
  y <<= 8;
  y |= Wire.read();
  y >>= 2;
  z = Wire.read();
  z <<= 8;
  z |= Wire.read();
  z >>= 2;

  uint8_t range = getRange();
  uint16_t divider = 1;
  if (range == MMA8451_RANGE_8_G) divider = 1024;
  if (range == MMA8451_RANGE_4_G) divider = 2048;
  if (range == MMA8451_RANGE_2_G) divider = 4096;

  x_g = (float) x / divider;
  y_g = (float) y / divider;
  z_g = (float) z / divider;
}

/**************************************************************************/
/*!
    @brief  Read the orientation:
    Portrait/Landscape + Up/Down/Left/Right + Front/Back
*/
/**************************************************************************/
uint8_t Adafruit_MMA8451::getOrientation(void) {
  return readRegister8(MMA8451_REG_PL_STATUS) & 0x07;
}

/**************************************************************************/
/*!
    @brief  Sets the g range for the accelerometer
*/
/**************************************************************************/
void Adafruit_MMA8451::setRange(mma8451_range_t range) {
  uint8_t reg1 = readRegister8(MMA8451_REG_CTRL_REG1);
  writeRegister8(MMA8451_REG_CTRL_REG1, 0x00);  // deactivate
  writeRegister8(MMA8451_REG_XYZ_DATA_CFG, range & 0x3);
  writeRegister8(MMA8451_REG_CTRL_REG1, reg1 | 0x01);  // activate
}

/**************************************************************************/
/*!
    @brief  Gets the g range for the accelerometer
*/
/**************************************************************************/
mma8451_range_t Adafruit_MMA8451::getRange(void) {
  /* Read the data format register to preserve bits */
  return (mma8451_range_t) (readRegister8(MMA8451_REG_XYZ_DATA_CFG) & 0x03);
}

/**************************************************************************/
/*!
    @brief  Sets the data rate for the MMA8451 (controls power consumption)
*/
/**************************************************************************/
void Adafruit_MMA8451::setDataRate(mma8451_dataRate_t dataRate) {
  uint8_t ctl1 = readRegister8(MMA8451_REG_CTRL_REG1);
  writeRegister8(MMA8451_REG_CTRL_REG1, 0x00);  // deactivate
  ctl1 &= ~(MMA8451_DATARATE_MASK << 3);        // mask off bits
  ctl1 |= (dataRate << 3);
  writeRegister8(MMA8451_REG_CTRL_REG1, ctl1 | 0x01);  // activate
}

/**************************************************************************/
/*!
    @brief  Gets the data rate for the MMA8451 (controls power consumption)
*/
/**************************************************************************/
mma8451_dataRate_t Adafruit_MMA8451::getDataRate(void) {
  return (mma8451_dataRate_t) ((readRegister8(MMA8451_REG_CTRL_REG1) >> 3) &
      MMA8451_DATARATE_MASK);
}

#ifdef USE_SENSOR
/**************************************************************************/
/*!
    @brief  Gets the most recent sensor event
*/
/**************************************************************************/
bool Adafruit_MMA8451::getEvent(sensors_event_t *event) {
  /* Clear the event */
  memset(event, 0, sizeof(sensors_event_t));

  event->version = sizeof(sensors_event_t);
  event->sensor_id = _sensorID;
  event->type = SENSOR_TYPE_ACCELEROMETER;
  event->timestamp = 0;

  read();

  // Convert Acceleration Data to m/s^2
  event->acceleration.x = x_g * SENSORS_GRAVITY_STANDARD;
  event->acceleration.y = y_g * SENSORS_GRAVITY_STANDARD;
  event->acceleration.z = z_g * SENSORS_GRAVITY_STANDARD;

  return true;
}

/**************************************************************************/
/*!
    @brief  Gets the sensor_t data
*/
/**************************************************************************/
void Adafruit_MMA8451::getSensor(sensor_t *sensor) {
  /* Clear the sensor_t object */
  memset(sensor, 0, sizeof(sensor_t));

  /* Insert the sensor name in the fixed length char array */
  strncpy(sensor->name, "MMA8451", sizeof(sensor->name) - 1);
  sensor->name[sizeof(sensor->name) - 1] = 0;
  sensor->version = 1;
  sensor->sensor_id = _sensorID;
  sensor->type = SENSOR_TYPE_ACCELEROMETER;
  sensor->min_delay = 0;
  sensor->max_value = 0;
  sensor->min_value = 0;
  sensor->resolution = 0;
}
#endif
