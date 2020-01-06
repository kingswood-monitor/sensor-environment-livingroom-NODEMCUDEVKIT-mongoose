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

Sensor *sensor;
bool hasCO2;

/**
 * Publish temperature, humidity, and CO2 
 * on a timer if active.
 */
static void timer_cb(void *)
{
  if (!sensor->isAvailable() || !mgos_sys_config_get_sensor_isActive()) return;

  std::string temp = sensor->temperatureString();
  mgos_mqtt_pub(temp_topic.c_str(), temp.c_str(), temp.size(), 1, 0);
  
  std::string humidity = sensor->humidityString();
  mgos_mqtt_pub(humidity_topic.c_str(), humidity.c_str(), humidity.size(), 1, 0);
  
  if (hasCO2)
  {
    std::string co2 = sensor->co2String();
    mgos_mqtt_pub(co2_topic.c_str(), co2.c_str(), co2.size(), 1, 0);
    LOG(LL_INFO, ("T: %s, H: %s, CO2: %s", temp.c_str(), humidity.c_str(), co2.c_str()));
  } else {
    LOG(LL_INFO, ("T: %s, H: %s", temp.c_str(), humidity.c_str()));
  }

  mgos_mqtt_pub(status_topic.c_str(), "ONLINE", 6, 1, 0);
  mgos_gpio_blink(LED_RED, update_millis - 50, 50);
}

enum mgos_app_init_result mgos_app_init(void)
{
  // Create sensor from configuration
  std::string sensor_name = std::string(mgos_sys_config_get_sensor_type());
  if (sensor_name == "HDC1080")
  {
    sensor = mgos_HDC1080_create();
    hasCO2 = mgos_sys_config_get_sensor_HDC1080_hasCO2();
    LOG(LL_INFO, ("Starting sensor %s", sensor_name.c_str()));
  }
  else if (sensor_name == "SCD30")
  {
    int refresh_secs = mgos_sys_config_get_sensor_SCD30_refreshIntervalSecs();
    int pressure_mbars = mgos_sys_config_get_sensor_SCD30_defaultAmbientPressureMbar();
    hasCO2 = mgos_sys_config_get_sensor_SCD30_hasCO2();
    sensor = mgos_SCD30_create(refresh_secs, pressure_mbars);
    LOG(LL_INFO, ("Starting sensor %s, refresh %i secs, pressure %i mbars", sensor_name.c_str(), refresh_secs, pressure_mbars));
  }
  else
  {
    LOG(LL_ERROR, ("Couldn't initialise sensor with name %s, check configuration.", sensor_name.c_str()));
    return MGOS_APP_INIT_ERROR;
  }

  // Set up the red onboard LED to signal MQTT transfer
  mgos_gpio_set_mode(LED_RED, MGOS_GPIO_MODE_OUTPUT);
  mgos_gpio_write(LED_RED, true); // true is off for onboard LED

  // Build the MQTT topic strings
  topic_root = std::string(mgos_sys_config_get_sensor_location()) + "/sensors/" + sensor_name;
  temp_topic = topic_root + "/temp";
  humidity_topic = topic_root + "/humidity";
  status_topic = topic_root + "/status";
  co2_topic = topic_root + "/co2";

  // Set the MQTT will topic
  mgos_sys_config_set_mqtt_will_topic(status_topic.c_str());

  // Start a simple repeating timer to scan data
  update_millis = mgos_sys_config_get_sensor_interval();
  mgos_set_timer(update_millis, true, timer_cb, NULL);

  return MGOS_APP_INIT_SUCCESS;
}
