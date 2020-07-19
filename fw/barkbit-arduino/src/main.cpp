#include <Adafruit_MMA8451.h>
#include <Adafruit_Sensor.h>
#include <Arduino.h>
#include <SEGGER_RTT.h>

#include "barkbit.h"
#include "bluefruit.h"
#include <Adafruit_LittleFS.h>
#include <InternalFileSystem.h>

using namespace Adafruit_LittleFS_Namespace;

#define rwrite_string SEGGER_RTT_WriteString
#define rprintf SEGGER_RTT_printf

#define PIN_MMA_INT1 2
#define PIN_MMA_INT2 3
#define PIN_FLASH_CS 7

Adafruit_MMA8451 mma = Adafruit_MMA8451();
SoftwareTimer ble_push;

BLEDis bledis;

transient_cfg_t transient_cfg{
    .x_enabled = true, .y_enabled = true, .z_enabled = true, .debounce_count = 1, .threshold = 10
};
uint32_t steps = 0;
bool update = false;

void save_cfg();

void threshold_write_cb(uint16_t conn_hdl, BLECharacteristic *chr, uint8_t *data, uint16_t len) {
  //
  if (*data != transient_cfg.threshold) {
    transient_cfg.threshold = *data;
    mma.setTransientConfiguration(&transient_cfg);
    save_cfg();
    rprintf(0, "threshold_write_cb: %d\n", *data);
  }
}

void debounce_write_cb(uint16_t conn_hdl, BLECharacteristic *chr, uint8_t *data, uint16_t len) {
  if (*data != transient_cfg.debounce_count) {
    transient_cfg.debounce_count = *data;
    mma.setTransientConfiguration(&transient_cfg);
    save_cfg();
    rprintf(0, "debounce_write_cb: %d\n", *data);
  }
}

void setup_bb_svc() {
  bb_service.begin();
  bb_steps.begin();
  bb_steps.setUserDescriptor("Steps");
  bb_steps.setProperties(CHR_PROPS_READ | CHR_PROPS_NOTIFY);
  bb_steps.setPermission(SECMODE_OPEN, SECMODE_NO_ACCESS);
  bb_steps.setFixedLen(sizeof(steps));
  bb_steps.begin();
  bb_steps.write32(steps);

  bb_threshold.begin();
  bb_threshold.setUserDescriptor("Threshold");
  bb_threshold.setProperties(CHR_PROPS_READ | CHR_PROPS_WRITE);
  bb_threshold.setPermission(SECMODE_OPEN, SECMODE_OPEN);
  bb_threshold.setFixedLen(1);
  bb_threshold.setWriteCallback(threshold_write_cb);
  bb_threshold.begin();
  bb_threshold.write8(transient_cfg.threshold);

  bb_debounce.begin();
  bb_debounce.setUserDescriptor("Debounce");
  bb_debounce.setProperties(CHR_PROPS_READ | CHR_PROPS_WRITE);
  bb_debounce.setPermission(SECMODE_OPEN, SECMODE_OPEN);
  bb_debounce.setFixedLen(1);
  bb_debounce.setWriteCallback(debounce_write_cb);
  bb_debounce.begin();
  bb_debounce.write8(transient_cfg.debounce_count);
}

void init_ble() {
  Bluefruit.configPrphBandwidth(BANDWIDTH_LOW);
  Bluefruit.begin();
  Bluefruit.setTxPower(0);  // TODO: tune this
  Bluefruit.setName("BarkBit");
  // bleuart.begin();

  bledis.setManufacturer("Gregus Archetectus");
  bledis.setModel("BarkBit 1");
  bledis.begin();

  setup_bb_svc();

  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addTxPower();
  Bluefruit.Advertising.addService(bb_service);
  Bluefruit.ScanResponse.addName();

  Bluefruit.Advertising.restartOnDisconnect(true);
  Bluefruit.Advertising.setInterval(200 / .625, 1000 / .625);
  Bluefruit.Advertising.setFastTimeout(5);  // number of seconds in fast mode
  Bluefruit.Advertising.start(0);  // 0 = Don't stop advertising after n seconds
}

void log() {
  mma.read();
  // rprintf(0, "MMA -> X:%d\tY:%d\tZ:%d.\n", mma.x, mma.y, mma.z);
  char buf[50];
  sprintf(buf, "MMA->Xg:%f\tYg:%f\tZg:%f.\n", mma.x_g, mma.y_g, mma.z_g);
  rwrite_string(0, buf);
}

void trans() {
  update = true;
  rwrite_string(0, "-------------------------------------------STEP-----\n");
  log();
  steps++;
}

void ble_push_cb(TimerHandle_t t) {
  if (update) {
    update = false;
    rwrite_string(0, "ble_push_cb\n");
    bb_steps.notify32(steps);
  }
}

const uint16_t cfg_size = 2;
const char * cfg_filename = "/bb.cfg";
void save_cfg() {
  File cfg_file(InternalFS);
  cfg_file.open(cfg_filename, FILE_O_WRITE);
  cfg_file.seek(0);
  cfg_file.write(transient_cfg.threshold);
  cfg_file.write(transient_cfg.debounce_count);
  cfg_file.close();
}

void load_cfg() {
  File cfg_file(InternalFS);
  cfg_file.open(cfg_filename, FILE_O_READ);
  if (!cfg_file || cfg_file.size() != cfg_size) {
    if(cfg_file)
    {
      rprintf(0, "Invalid config size of %d. Resetting.\n", cfg_file.size());
      InternalFS.remove(cfg_filename);
    } else {
      rwrite_string(0, "Config file not found. Resetting.\n");
    }
    save_cfg();
  } else {
    cfg_file.readBytes(&transient_cfg.threshold, 1);
    cfg_file.readBytes(&transient_cfg.debounce_count, 1);
    cfg_file.close();
  }
}

void setup() {
  pinMode(PIN_FLASH_CS, OUTPUT);
  digitalWrite(PIN_FLASH_CS, HIGH);

  InternalFS.begin();
  load_cfg();

  sd_power_dcdc_mode_set(NRF_POWER_DCDC_ENABLE);

  while (!mma.begin(0x1C, &transient_cfg)) {
    rwrite_string(0, "MMA8451: couldnt start\n");
    delay(1000);
  }
  rprintf(0, "MMA8451 initialized with t:%d, d:%d!\n", transient_cfg.threshold, transient_cfg.debounce_count);

  Serial.end();

  sd_power_mode_set(NRF_POWER_MODE_LOWPWR);

  // pinMode(PIN_MMA_INT1, INPUT_PULLUP);
  // attachInterrupt(digitalPinToInterrupt(PIN_MMA_INT1), log, FALLING);

  init_ble();

  pinMode(PIN_MMA_INT2, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(PIN_MMA_INT2), trans, FALLING);

  ble_push.begin(5000, ble_push_cb);
  ble_push.start();

  suspendLoop();
  waitForEvent();
}

void loop() {}
