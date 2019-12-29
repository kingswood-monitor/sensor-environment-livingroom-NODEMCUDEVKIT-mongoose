#include <string>
#include "sensor.h"

Sensor::Sensor()
{
}

void Sensor::setType(std::string type_name)
{
    m_type_name = type_name;
}

std::string Sensor::getType()
{
    return m_type_name;
}

bool Sensor::initialise()
{
    if (m_type_name == "SCD30")
    {
        return true;
    }
    else
    {
        return false;
    }
}

float Sensor::readTemperature()
{
    return 21.0;
}

float Sensor::readHumidity()
{
    return 0.1;
}

int Sensor::readCO2()
{
    return 420;
}

bool Sensor::isAvailable()
{
    return true;
}
