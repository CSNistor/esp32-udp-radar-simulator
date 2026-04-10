#include <cstdint>
#include "esp_log.h"
#include "WiFi.hpp"
#include "UdpSender.hpp"
#include "nvs_flash.h"

constexpr const char *WIFI_SSID = "YOUR_WIFI_SSID";
constexpr const char *WIFI_PASS = "YOUR_WIFI_PASSWORD";
constexpr const char *DEST_IP = "DESTINATION_IP";
constexpr uint16_t DEST_PORT = 5000;

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
    ESP_LOGI(TAG, "initializing Wi-Fi");
    static radar::WiFiStation wifi(WIFI_SSID, WIFI_PASS);
    if (!wifi.connect())
    {
        ESP_LOGE(TAG, "Wi-Fi connection failed");
        return;
    }

    static radar::UdpSender sender(DEST_IP, DEST_PORT);

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
