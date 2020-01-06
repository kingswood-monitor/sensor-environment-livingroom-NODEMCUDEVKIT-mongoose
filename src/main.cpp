/*
 * Livingroom Enviromnent Sensor firmware
 * sensor-environment-livingroom-mongoose
 * 
 * Kingswood Environment Sensor Firmware
 * Mongoose OS
 * 
 * Reads temperature, humidity from DHT22
 * Publishes to MQTT topic
 * 
 * Indicators:
 * -----------------------------------------
 * Solid blue LED - WiFi connected
 * Flashing red LED - MQTT server  connected, data transmitted
 * 
 *  JSON V1.0
 
    packetID = 1234
    protocol n = 1.1
    device
      ID = ESP8266-001
      type = ESP8266
      location = livingroom
      firmware n = 1.1 
      os = mongoose
      battery
        active b = true
        voltage n = 3.8
      lora
        RSSI n = -98
        SNR n = 23
        frequencyError n = 12234
    sensors
      SHT15
        temperature n = 21.1
        humidity n = 87
      BH1750
        lux n = 600
      BMP388
        mbars n = 1023
    status 
      message = OK
      description = All's well
 * 
 */
#include <string>
#include "mgos.h"
#include "mgos_wifi.h"
#include "mgos_mqtt.h"
#include "mgos_gpio.h"
#include "mgos_provision.h"

#include "mgos_arduino_sparkfun_scd30.h"
#include "mgos_arduino_closedcube_hdc1080.h"

#include "sensor-utils.h"

// Red  Onboard LED next to USB port, used to signal data transmission
#define LED_RED 16

// MQTT update interval, milliseconds
int update_millis;

// MQTT topic strings
std::string topic_root;
std::string temp_topic;
std::string humidity_topic;
std::string status_topic;
std::string co2_topic;
std::string message_topic;

Sensor *sensor;
bool hasCO2;

/**
 * Publish temperature, humidity, and CO2 
 * on a timer if active.
 */
static void timer_cb(void *)
{
  if (!sensor->isAvailable()) return;

  std::string temp = sensor->temperatureString();
  std::string humidity = sensor->humidityString();
  std::string co2 = sensor->co2String();
  
  mgos_mqtt_pub(temp_topic.c_str(), temp.c_str(), temp.size(), 1, 0);
  mgos_mqtt_pub(humidity_topic.c_str(), humidity.c_str(), humidity.size(), 1, 0);
  mgos_mqtt_pub(co2_topic.c_str(), co2.c_str(), co2.size(), 1, 0);

  LOG(LL_INFO, ("T: %s, H: %s, CO2: %s", temp.c_str(), humidity.c_str(), co2.c_str()));

  mgos_mqtt_pub(status_topic.c_str(), "ONLINE", 6, 1, 0);
  mgos_gpio_blink(LED_RED, update_millis - 50, 50);
}

enum mgos_app_init_result mgos_app_init(void)
{
  const char *firmware_title = mgos_sys_config_get_device_firmware_title();
  const char *firmware_version = mgos_sys_config_get_device_firmware_version();
  const char *device_os = mgos_sys_config_get_device_os();
  const char *firmware_filename = mgos_sys_config_get_device_firmware_filename();
  const char *json_version = mgos_sys_config_get_json_protocol_version();
  const char *device_id = mgos_sys_config_get_device_id(); // TODO FIXME get actual device id

  utils::printBanner(firmware_title, firmware_version, device_os, firmware_filename, json_version, device_id);

  // Create sensor from configuration
  // sensor = mgos_HDC1080_create();
  int refresh_secs = mgos_sys_config_get_sensor_scd30_refreshIntervalSecs();
  int pressure_mbars = mgos_sys_config_get_sensor_scd30_defaultAmbientPressureMbar();
  sensor = mgos_SCD30_create(refresh_secs, pressure_mbars);
  
  // Set up the red onboard LED to signal MQTT transfer
  mgos_gpio_set_mode(LED_RED, MGOS_GPIO_MODE_OUTPUT);
  mgos_gpio_write(LED_RED, true); // true is off for onboard LED

  // Build the MQTT topic strings
  topic_root = std::string(mgos_sys_config_get_device_location());
  temp_topic = topic_root + "/sensors/scd30/temp";
  humidity_topic = topic_root + "/sensors/scd30/humidity";
  co2_topic = topic_root + "/sensors/scd30/co2";
  message_topic = topic_root + "/status/message";

  // Set the MQTT will topic
  mgos_sys_config_set_mqtt_will_topic(status_topic.c_str());

  // Start a simple repeating timer to scan data
  update_millis = mgos_sys_config_get_sensor_interval();
  mgos_set_timer(update_millis, true, timer_cb, NULL);

  return MGOS_APP_INIT_SUCCESS;
}
