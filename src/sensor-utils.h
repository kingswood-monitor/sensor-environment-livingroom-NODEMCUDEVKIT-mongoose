#include "mgos.h"

namespace utils 
{
void printBanner(const char *firmware_title, const char *firmware_version, const char *device_os, 
                 const char *firmware_filename, const char *json_protocol_version, const char *deviceID)
{

    LOG(LL_INFO, ("======================================================================%d", NULL));
    LOG(LL_INFO, ("# %s V%s", firmware_title, firmware_version));
    LOG(LL_INFO, ("----------------------------------------------------------------------%d",  NULL));
    LOG(LL_INFO, ("Firmware filename : %s", firmware_filename));
    LOG(LL_INFO, ("Operating system  : %s", device_os));
    LOG(LL_INFO, ("Protocol version  : %s", json_protocol_version));
    LOG(LL_INFO, ("Device ID         : %s", deviceID));
    LOG(LL_INFO, ("=======================================================================\n\n%d", NULL));
}


} // namespace