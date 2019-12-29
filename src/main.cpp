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
 *  - Convert SCD30 driver
 *  - Build generic sensor driver
 *  - Check I2C SCL/SDA pins for correctness
 *  - Add NeoPixel support
 *  - Build Blinkt app
 * 
 * DONE:
 *  - Publish on MQTT
 *  - Flash blue LED on WIFI connect
 *  - FLash red LED on data send
 *  - Convert to HDC1080 sensor
 *  - Parameterise MQTT topic
 *  - Integrate with Node-Red code (inc. MQTT status / wil)
 * 
 */
#include <string>
#include "mgos.h"
#include "mgos_wifi.h"
#include "mgos_mqtt.h"
#include "mgos_gpio.h"
#include "mgos_provision.h"
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

// The sensor
ClosedCube_HDC1080 *hdc1080 = mgos_HDC1080_create();

// Main loop timer callback
static void
timer_cb(void *)
{
  // If connected to network, convert readings to string and publish to MQTT
  if ((mgos_HDC1080_is_connected(hdc1080)) && (mgos_sys_config_get_sensor_isActive()))
  {
    double temp_c = mgos_HDC1080_read_temperature(hdc1080);
    double humidity = mgos_HDC1080_read_humidity(hdc1080);

    // Publish readings to MQTT
    char buffer[50];
    int len;
    len = snprintf(buffer, 50, "%f", temp_c);
    mgos_mqtt_pub(temp_topic.c_str(), buffer, len, 1, 0);
    len = snprintf(buffer, 50, "%f", humidity);
    mgos_mqtt_pub(humidity_topic.c_str(), buffer, len, 1, 0);

    // Publish state to MQTT
    mgos_mqtt_pub(status_topic.c_str(), "ONLINE", 6, 1, 0);

    // Log and blink
    mgos_gpio_blink(LED_RED, update_millis - 50, 50);
    LOG(LL_INFO, ("T: %.2f, H: %.2f", temp_c, humidity));
  }
}

enum mgos_app_init_result mgos_app_init(void)
{
  // Initialise the sensor
  mgos_HDC1080_begin(hdc1080);

  // Set up the red onboard LED to signal MQTT transfer
  mgos_gpio_set_mode(LED_RED, MGOS_GPIO_MODE_OUTPUT);
  mgos_gpio_write(LED_RED, true); // true is off for onboard LED

  // Build the MQTT topic strings
  topic_root = std::string(mgos_sys_config_get_sensor_topicroot());
  temp_topic = topic_root + "/temp";
  humidity_topic = topic_root + "/humidity";
  status_topic = topic_root + "/status";

  // Set the MQTT will topic
  mgos_sys_config_set_mqtt_will_topic(status_topic.c_str());

  // Start a simple repeating timer to scan data
  update_millis = mgos_sys_config_get_sensor_interval();
  mgos_set_timer(update_millis, true, timer_cb, NULL);

  return MGOS_APP_INIT_SUCCESS;
}
