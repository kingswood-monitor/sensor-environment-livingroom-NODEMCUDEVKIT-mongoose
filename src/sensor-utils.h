#include "mgos.h"

namespace utils
{
/** Configuration file: 
 * 
 *  device
 *      firmware_title
 *      firmware_version
 *      firmware_filename
 *      os
 *  json
 *      protocol_version    
 */
void printBanner()
{
    const char *firmware_name = mgos_sys_config_get_device_firmware_name();
    const char *device_location = mgos_sys_config_get_device_location();
    const char *firmware_slug = mgos_sys_config_get_device_firmware_slug();
    const char *firmware_version = mgos_sys_config_get_device_firmware_version();
    const char *firmware_mcu = mgos_sys_config_get_device_type();
    const char *firmware_os = mgos_sys_config_get_device_firmware_os();
    const char *device_id = mgos_sys_config_get_device_id(); // TODO FIXME get actual device id

    LOG(LL_INFO, ("======================================================================%d", NULL));
    LOG(LL_INFO, ("# %s (%s)", firmware_name, device_location));
    LOG(LL_INFO, ("----------------------------------------------------------------------%d", NULL));
    LOG(LL_INFO, ("Firmware  : %s v%s", firmware_slug, firmware_version));
    LOG(LL_INFO, ("MCU       : %s", firmware_mcu));
    LOG(LL_INFO, ("OS        : %s", firmware_os));
    LOG(LL_INFO, ("Device ID : %s", device_id));
    LOG(LL_INFO, ("=======================================================================\n\n%d", NULL));
}

} // namespace utils