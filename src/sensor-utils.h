#include "mgos.h"

namespace utils
{
/** Configuration file: 
 * 
 *  device
 *      id
 *      firmware_title
 *      firmware_version
 *      firmware_filename
 *      os
 *  json
 *      protocol_version    
 */     
void printBanner()
{
    const char *firmware_title = mgos_sys_config_get_device_firmware_title();
    const char *firmware_version = mgos_sys_config_get_device_firmware_version();
    const char *device_os = mgos_sys_config_get_device_os();
    const char *firmware_filename = mgos_sys_config_get_device_firmware_filename();
    const char *json_version = mgos_sys_config_get_json_protocol_version();
    const char *device_id = mgos_sys_config_get_device_id(); // TODO FIXME get actual device id

    LOG(LL_INFO, ("======================================================================%d", NULL));
    LOG(LL_INFO, ("# %s V%s", firmware_title, firmware_version));
    LOG(LL_INFO, ("----------------------------------------------------------------------%d", NULL));
    LOG(LL_INFO, ("Firmware filename : %s", firmware_filename));
    LOG(LL_INFO, ("Operating system  : %s", device_os));
    LOG(LL_INFO, ("Protocol version  : %s", json_version));
    LOG(LL_INFO, ("Device ID         : %s", device_id));
    LOG(LL_INFO, ("=======================================================================\n\n%d", NULL));
}

} // namespace utils