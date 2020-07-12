#include <Arduino.h>
#include <SEGGER_RTT.h>
#include "bluefruit.h"
#include <Adafruit_MMA8451.h>
#include <Adafruit_Sensor.h>

#define rwrite_string SEGGER_RTT_WriteString
#define rprintf SEGGER_RTT_printf

#define MMA_SLOW MMA8451_DATARATE_1_56_HZ
#define MMA_FAST MMA8451_DATARATE_12_5_HZ

Adafruit_MMA8451 mma = Adafruit_MMA8451();
SoftwareTimer mma_read;

BLEDis bledis;
BLEUart bleuart;

#define DEFAULT_HIGHRATE_TIMEOUT 5000
#define AVG_READINGS 50

int last = 0;
mma8451_dataRate_t current_rate = MMA_SLOW;
int high_rate_timeout = 0;
int16_t xavg = 439, yavg = 330, zavg = 1967;
int16_t readings[AVG_READINGS][3] = {0};
uint8_t reading_number = 0;

void recalc_averages()
{
  uint32_t xsum = 0, ysum = 0, zsum = 0;
  for (uint8_t i = 0; i < AVG_READINGS; i++)
  {
    xsum = xsum + abs(readings[i][0]);
    ysum = ysum + abs(readings[i][1]);
    zsum = zsum + abs(readings[i][2]);
  }

  reading_number = 0;
  xavg = roundf(xsum / 50);
  yavg = roundf(ysum / 50);
  zavg = roundf(zsum / 50);

  rprintf(0, "NEW AVERAGES -> X:%u\tY:%u\tZ:%u.\n", xavg, yavg, zavg);
}

void read_sensor(TimerHandle_t xTimer)
{
  mma.read();
  int now = abs(last - (mma.x + mma.y + mma.z));
  if (now > 5)
  {
    high_rate_timeout = millis();
  }

  readings[reading_number][0] = mma.x;
  readings[reading_number][1] = mma.y;
  readings[reading_number][2] = mma.z;
  reading_number++;
  if (reading_number == AVG_READINGS)
  {
    recalc_averages();
  }

  int totvect = sqrt(((abs(mma.x) - xavg) ^ 2) + ((abs(mma.y) - yavg) ^ 2) + ((abs(mma.z) - zavg) ^ 2));
  // totave[a] = (totvect[a] + totvect[a - 1]) / 2 ;
  if (totvect > 20)
  {
    rwrite_string(0, "-------------------------------------------------STEP-----\n");
    bleuart.println("<- STEP ->");
    xavg = abs(mma.x);
    yavg = abs(mma.y);
    zavg = abs(mma.z);
  }

  last = mma.x + mma.y + mma.z;
  rprintf(0, "MMA l:(%d),t:(%d) -> X:%d\tY:%d\tZ:%d.\n", now, totvect, mma.x, mma.y, mma.z);

  if (current_rate == MMA_SLOW || reading_number % 10 == 0)
  {
    bleuart.printf("l:(%d),t:(%d)", now, totvect);
    bleuart.println();
  }

  if (now > 5 && current_rate != MMA_FAST)
  {
    rwrite_string(0, "Switching to 12.5hz.\n");
    current_rate = MMA_FAST;
    mma.setDataRate(MMA_FAST);
    mma_read.setPeriod(1000 / 12.5);
  }
  else if (current_rate != MMA_SLOW && (millis() - high_rate_timeout) > DEFAULT_HIGHRATE_TIMEOUT)
  {
    rwrite_string(0, "Switching to 1.5hz.\n");
    current_rate = MMA_SLOW;
    mma.setDataRate(MMA_SLOW);
    mma_read.setPeriod(1000 / 1.5);
  }
}

void init_ble()
{
  Bluefruit.configPrphBandwidth(BANDWIDTH_LOW);
  Bluefruit.begin();
  Bluefruit.setTxPower(0); // TODO: tune this
  Bluefruit.setName("BarkBit");
  bleuart.begin();
  bledis.begin();

  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addTxPower();
  Bluefruit.Advertising.addService(bleuart);
  Bluefruit.ScanResponse.addName();

  /* Start Advertising
     * - Enable auto advertising if disconnected
     * - Interval:  fast mode = 20 ms, slow mode = 152.5 ms
     * - Timeout for fast mode is 30 seconds
     * - Start(timeout) with timeout = 0 will advertise forever (until connected)
     *
     * For recommended advertising interval
     * https://developer.apple.com/library/content/qa/qa1931/_index.html
     */
  Bluefruit.Advertising.restartOnDisconnect(true);
  Bluefruit.Advertising.setInterval(32, 244); // in unit of 0.625 ms
  Bluefruit.Advertising.setFastTimeout(30);   // number of seconds in fast mode
  Bluefruit.Advertising.start(0);             // 0 = Don't stop advertising after n seconds
}

void setup()
{
  while (!mma.begin(0x1C))
  {
    rwrite_string(0, "MMA8451: couldnt start\n");
    delay(1000);
  }
  rwrite_string(0, "MMA8451 found!\n");
  mma.setDataRate(MMA_SLOW);

  //Serial.begin(115200);
  //  Bluefruit.begin();
  //  Bluefruit.setTxPower(0);
  //
  //  sd_power_mode_set(NRF_POWER_MODE_LOWPWR);
  //sd_softdevice_disable();
  //sd_power_dcdc_mode_set(NRF_POWER_DCDC_ENABLE);

  init_ble();

  mma_read.begin(1000 / 12, read_sensor);
  mma_read.start();
  suspendLoop();
}

void loop()
{

  //SEGGER_RTT_printf(0, "BarkBit -> X:%d\tY:%d\tZ:%d.\n", mma.x, mma.y, mma.z);
  //sd_power_mode_set(NRF_POWER_MODE_LOWPWR);
  delay(20);
}
