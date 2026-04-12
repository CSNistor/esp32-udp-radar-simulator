#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <cstdint>

namespace radar::config
{
    constexpr const char *WIFI_SSID = "";
    constexpr const char *WIFI_PASS = "";
    constexpr const char *DEST_IP = "192.168.1.100";
    constexpr uint16_t DEST_PORT = 5000;
}

#endif // CONFIG_HPP