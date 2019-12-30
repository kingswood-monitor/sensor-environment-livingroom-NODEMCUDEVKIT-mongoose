/*
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
 * TODO:
 *  - Add NeoPixel support
 *  - Build Blinkt app
 *  - Write native I2C HDC1080 / SCD30 libraries
 * 
 * DONE:
 *  - Publish on MQTT
 *  - Flash blue LED on WIFI connect
 *  - FLash red LED on data send
 *  - Convert to HDC1080 sensor
 *  - Parameterise MQTT topic
 *  - Integrate with Node-Red code (inc. MQTT status / wil)
 *  - Convert SCD30 driver
 *  - Build generic sensor driver
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

// Main loop timer callback
static void
timer_cb(void *)
{
  // If connected to network, convert readings to string and publish to MQTT
  if (sensor->isAvailable() && mgos_sys_config_get_sensor_isActive())
  {
    double temp_c = sensor->readTemperature();
    double humidity = sensor->readHumidity();
    int co2 = sensor->readCO2();

    // Publish readings to MQTT
    char buffer[50];
    int len;
    // temperature
    len = snprintf(buffer, 50, "%f", temp_c);
    mgos_mqtt_pub(temp_topic.c_str(), buffer, len, 1, 0);
    // humdity
    len = snprintf(buffer, 50, "%f", humidity);
    mgos_mqtt_pub(humidity_topic.c_str(), buffer, len, 1, 0);
    // co2
    if (hasCO2)
    {
      len = snprintf(buffer, 50, "%i", co2);
      mgos_mqtt_pub(co2_topic.c_str(), buffer, len, 1, 0);
    }

    // Publish state to MQTT
    mgos_mqtt_pub(status_topic.c_str(), "ONLINE", 6, 1, 0);

    // Log and blink
    if (hasCO2)
    {
      LOG(LL_INFO, ("T: %.2f, H: %.2f, CO2: %i", temp_c, humidity, co2));
    }
    else
    {
      LOG(LL_INFO, ("T: %.2f, H: %.2f", temp_c, humidity));
    }
    mgos_gpio_blink(LED_RED, update_millis - 50, 50);
  }
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
  }

  // Set up the red onboard LED to signal MQTT transfer
  mgos_gpio_set_mode(LED_RED, MGOS_GPIO_MODE_OUTPUT);
  mgos_gpio_write(LED_RED, true); // true is off for onboard LED

  // Build the MQTT topic strings
  topic_root = std::string(mgos_sys_config_get_sensor_topicroot());
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
