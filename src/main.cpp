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
 *  Format
 *  ------------------------------------------------------------
    packetID = 1234
    protocol n = 1.1

    device
      ID = ESP8266-001
      type = ESP8266
      firmware
        version n = 0.2
        slug = sensor-environment-outside-32U4RFM95LORA-arduino
        os = mongoose
      battery
        active b = true
        voltage n = 3.8
      lora
        RSSI n = -98
        SNR n = 23
        frequencyError n = 12234

    measurement
      temperature n = 21.1
      humidity n = 87
      co2 = 567
      lux n = 600
      mbars n = 1023
        
    status 
      message = OK
      description = All's well
    ---------------------------------------------------------------
 * 
 */
#include <string>
#include "mgos.h"
#include "mgos_wifi.h"
#include "mgos_provision.h"
#include "mgos_mqtt.h"
#include "mgos_gpio.h"

#include "mgos_arduino_sparkfun_scd30.h"
#include "mgos_arduino_closedcube_hdc1080.h"

#include "sensor-utils.h"

// Red  Onboard LED next to USB port, used to signal data transmission
#define LED_RED 16

// MQTT update interval, milliseconds
int update_millis;

// MQTT topic strings
std::string root_tp;
std::string packetid_tp;
std::string protocol_tp;

std::string device_id_tp;
std::string device_type_tp;
std::string device_location_tp;
std::string device_firmware_tp;
std::string device_os_tp;
std::string device_battery_active_tp;
std::string device_battery_voltage_tp;

std::string measurement_temperature_tp;
std::string measurement_humidity_tp;
std::string measurement_co2_tp;
std::string measurement_lux_tp;
std::string measurement_mbars_tp;

std::string status_message_tp;
std::string status_description_tp;

//MQTT values
const char *device_id;
const char *device_type;
const char *device_location;
const char *device_firmware;
const char *device_os;
const char *device_battery_active;
const char *device_battery_voltage;

Sensor *sensor;
bool hasCO2;

/**
 * Publish temperature, humidity, and CO2 
 * on a timer if active.
 */
static void timer_cb(void *) {
    if (!sensor->isAvailable())
        return;

    std::string temp = sensor->temperatureString();
    std::string humidity = sensor->humidityString();
    std::string co2 = sensor->co2String();

    mgos_mqtt_pub(status_message_tp.c_str(), "ONLINE", 6, 1, 0);

    mgos_mqtt_pub(measurement_temperature_tp.c_str(), temp.c_str(), temp.size(), 1, 0);
    mgos_mqtt_pub(measurement_humidity_tp.c_str(), humidity.c_str(), humidity.size(), 1, 0);
    mgos_mqtt_pub(measurement_co2_tp.c_str(), co2.c_str(), co2.size(), 1, 0);

    LOG(LL_INFO, ("T: %s, H: %s, CO2: %s", temp.c_str(), humidity.c_str(), co2.c_str()));
    mgos_gpio_blink(LED_RED, update_millis - 50, 50);
}

enum mgos_app_init_result mgos_app_init(void) {

    utils::printBanner();

    // Create sensor from configuration
    // sensor = mgos_HDC1080_create();
    int refresh_secs = mgos_sys_config_get_sensor_scd30_refreshIntervalSecs();
    int pressure_mbars = mgos_sys_config_get_sensor_scd30_defaultAmbientPressureMbar();
    sensor = mgos_SCD30_create(refresh_secs, pressure_mbars);

    // Set up the red onboard LED to signal MQTT transfer
    mgos_gpio_set_mode(LED_RED, MGOS_GPIO_MODE_OUTPUT);
    mgos_gpio_write(LED_RED, true); // true is off for onboard LED

    // Build the MQTT topic strings
    root_tp = std::string(mgos_sys_config_get_mqtt_topic_root());

    packetid_tp = root_tp + "/packetID";

    device_id_tp = root_tp + "/device/id";
    device_type_tp = root_tp + "/device/type";
    device_os_tp = root_tp + "/device/firmware/os";
    device_battery_active_tp = root_tp + "/device/battery/active";
    device_battery_voltage_tp = root_tp + "/device/battery/voltage";

    measurement_temperature_tp = root_tp + "/measurement/temperature";
    measurement_humidity_tp = root_tp + "/measurement/humidity";
    measurement_co2_tp = root_tp + "/measurement/co2";
    measurement_lux_tp = root_tp + "/measurement/lux";
    measurement_mbars_tp = root_tp + "/measurement/mbars";

    status_message_tp = root_tp + "/status/message";
    status_description_tp = root_tp + "/status/description";

    // Set the MQTT will topic
    mgos_sys_config_set_mqtt_will_topic(status_message_tp.c_str());

    // MQTT values
    device_id = mgos_sys_config_get_device_id();
    device_type = mgos_sys_config_get_device_type();
    device_location = mgos_sys_config_get_device_location();
    device_firmware = mgos_sys_config_get_device_firmware_name();
    device_os = mgos_sys_config_get_device_firmware_os();
    device_battery_active = NULL;
    device_battery_voltage = NULL;

    // Start a simple repeating timer to scan data
    update_millis = mgos_sys_config_get_sensor_interval();
    mgos_set_timer(update_millis, true, timer_cb, NULL);

    return MGOS_APP_INIT_SUCCESS;
}
