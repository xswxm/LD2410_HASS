#include "esphome.h"

static const char *TAG = "ld2410";
static const uint8_t RESPONSE_LENGTH = 23;

using namespace esphome;

class LD2410Sensor : public PollingComponent,  public UARTDevice, public sensor::Sensor{
 public:
  LD2410Sensor(UARTComponent *parent) : PollingComponent(1000), UARTDevice(parent) {}

  Sensor *target_sensor = new Sensor();
  Sensor *distance_sensor = new Sensor();
  Sensor *energy_sensor = new Sensor();
  Sensor *distance_standby_sensor = new Sensor();
  Sensor *energy_standby_sensor = new Sensor();
  Sensor *range_sensor = new Sensor();

  void setup() override {
      ESP_LOGCONFIG(TAG, "Setting up LD2410...");
  }

  void loop() override {}

  void update() override {
    uint8_t response[RESPONSE_LENGTH];
    
    flush();
    bool read_success = read_array(response, RESPONSE_LENGTH);
    flush();
    
    if (!read_success) {
      ESP_LOGW(TAG, "Reading data from LD2410 failed!");
      status_set_warning();
      return;
    }

    uint8_t i;
    uint8_t shift_step;
    uint8_t response_temp[RESPONSE_LENGTH];

    for (shift_step = 0; shift_step < RESPONSE_LENGTH; shift_step++) {
      if (response[shift_step] == 0xF4 && response[(shift_step + 1) % (RESPONSE_LENGTH - 1)] == 0xF3 && response[(shift_step + 2) % (RESPONSE_LENGTH - 1)] == 0xF2 && response[(shift_step + 3) % (RESPONSE_LENGTH - 1)] == 0xF1) {
        break;
      }
    }
    if (shift_step == RESPONSE_LENGTH) {
      ESP_LOGW(TAG, "Invalid preamble(LEN %02X): 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X", RESPONSE_LENGTH, response[0], response[1], response[2], response[3], response[4], response[5], response[6], response[7], response[8], response[9], response[10], response[11], response[12], response[13], response[14], response[15], response[16], response[17], response[18], response[19], response[20], response[21], response[22]);
      this->status_set_warning();
      return;
    }
    else if (shift_step > 0) {
      ESP_LOGW(TAG, "Data Shift. Step: %02X", shift_step);
      for(i = 0; i < shift_step; i++) {
        response_temp[i] = response[i];
      }
      for(i = 0; i < RESPONSE_LENGTH - shift_step; i++) {
        response[i] = response[i + shift_step];
      }
      for (i = 0; i < shift_step; i++) {
        response[i + RESPONSE_LENGTH - shift_step] = response_temp[i];
      }
    }

    if (response[6] != 0x02) {
      ESP_LOGW(TAG, "Mode doesn't support: 0x%02X", response[6]);
      this->status_set_warning();
      return;
    }

    if (response[4] != 0x0D) {
      ESP_LOGW(TAG, "Data length doesn't match: 0x%02X!=0x%02X", response[4], 0x0D);
      this->status_set_warning();
      return;
    }

    if (response[18] != 0x00) {
      ESP_LOGW(TAG, "Checksum doesn't match: 0x%02X!=0x%02X", response[18], 0x00);
      this->status_set_warning();
      return;
    }

    this->status_clear_warning();

    ESP_LOGW(TAG, "Successfully read LD2410 data");

    const uint16_t target = response[8];
    const uint16_t distance = (response[10] * 256) + response[9];
    const uint16_t energy = response[11];
    const uint16_t distance_standby = (response[13] * 256) + response[12];
    const uint16_t energy_standby = response[14];
    const uint16_t range = (response[16] * 256) + response[15];

    ESP_LOGD(TAG, "Target status: %u", target);
    this->target_sensor->publish_state(target);

    ESP_LOGD(TAG, "Distance: %u cm", distance);
    this->distance_sensor->publish_state(distance);

    ESP_LOGD(TAG, "Energy: %u %%", energy);
    this->energy_sensor->publish_state(energy);

    ESP_LOGD(TAG, "Distance (Standby): %u cm", distance_standby);
    this->distance_standby_sensor->publish_state(distance_standby);

    ESP_LOGD(TAG, "Energy (Standby): %u %%", energy_standby);
    this->energy_standby_sensor->publish_state(energy_standby);

    ESP_LOGD(TAG, "Range: %u cm", range);
    this->range_sensor->publish_state(range);
  }

};
