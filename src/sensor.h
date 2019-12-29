/*
 * Virtual environment sensor
 * 
 * Abstracts the physical HDC1080 and SDC30 sensors to provide a unified interface for the firmware.
 * 
 * Provides the following methods:
 * 
 * Readings readings = getReadings();
 */
#include <string>

class Sensor
{
private:
    std::string m_type_name;

public:
    Sensor();
    bool initialise();
    void setType(std::string type_name);
    std::string getType();
    float readTemperature();
    float readHumidity();
    int readCO2();
    bool isAvailable();
};