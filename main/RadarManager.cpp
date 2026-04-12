#include <cstdint>
#include "esp_log.h"
#include "WiFi.hpp"
#include "UdpSender.hpp"
#include "nvs_flash.h"
#include "Config.hpp"

extern "C" void app_main(void)
{
    static const char *TAG = "RadarManager";
    ESP_LOGI(TAG, "boot start");
    // Initialize NVS
    ESP_LOGI(TAG, "initializing NVS");
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    ESP_LOGI(TAG, "NVS succesfully initialized!");

    if (radar::config::WIFI_SSID[0] == '\0')
    {
        ESP_LOGE(TAG, "Wi-Fi SSID is empty");
        return;
    }

    if (radar::config::WIFI_PASS[0] == '\0')
    {
        ESP_LOGE(TAG, "Wi-Fi password is empty");
        return;
    }

    if (radar::config::DEST_IP[0] == '\0')
    {
        ESP_LOGE(TAG, "Destination IP is empty");
        return;
    }

    if (radar::config::DEST_PORT == 0)
    {
        ESP_LOGE(TAG, "Destination port is invalid");
        return;
    }

    ESP_LOGI(TAG, "initializing Wi-Fi");
    static radar::WiFiStation wifi(
        radar::config::WIFI_SSID,
        radar::config::WIFI_PASS);
    if (!wifi.connect())
    {
        ESP_LOGE(TAG, "Wi-Fi connection failed");
        return;
    }

    static radar::UdpSender sender(
        radar::config::DEST_IP,
        radar::config::DEST_PORT);

    if (!sender.init())
    {
        ESP_LOGE(TAG, "UDP sender init failed");
        return;
    }

    if (!sender.startTask())
    {
        ESP_LOGE(TAG, "UDP sender task start failed");
        return;
    }
}
