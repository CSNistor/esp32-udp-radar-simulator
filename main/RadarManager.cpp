#include <cstdint>

#include "esp_log.h"
#include "nvs_flash.h"
#include "freertos/queue.h"

#include "Config.hpp"
#include "WiFi.hpp"
#include "RadarSample.hpp"
#include "RadarProducer.hpp"
#include "UdpSender.hpp"

extern "C" void app_main(void)
{
    static const char *TAG = "RadarManager";

    ESP_LOGI(TAG, "boot start");
    ESP_LOGI(TAG, "initializing NVS");

    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_LOGI(TAG, "NVS successfully initialized!");

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

    QueueHandle_t xStructQueue = xQueueCreate(10, sizeof(radar::RadarSample));

    if (xStructQueue == NULL)
    {
        ESP_LOGE(TAG, "Failed to create radar sample queue");
        return;
    }

    ESP_LOGI(TAG, "Radar sample queue created");

    static radar::RadarProducer producer(xStructQueue);
    static radar::UdpSender sender(
        radar::config::DEST_IP,
        radar::config::DEST_PORT,
        xStructQueue);

    if (!sender.init())
    {
        ESP_LOGE(TAG, "UDP sender init failed");
        return;
    }

    if (!producer.startTask())
    {
        ESP_LOGE(TAG, "RadarProducer start failed");
        return;
    }

    if (!sender.startTask())
    {
        ESP_LOGE(TAG, "UDP sender task start failed");
        return;
    }

    ESP_LOGI(TAG, "RadarManager startup completed successfully");
}
