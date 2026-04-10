#include "WiFi.hpp"

#include <cstring>

#include "esp_check.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "lwip/inet.h"

namespace radar
{
    WiFiStation::WiFiStation(const char *ssid, const char *password, int maxRetries)
        : ssid_(ssid),
          password_(password),
          maxRetries_(maxRetries),
          retryCount_(0),
          connected_(false),
          wifiEventGroup_(nullptr),
          wifiEventInstance_(nullptr),
          ipEventInstance_(nullptr)
    {
    }

    WiFiStation::~WiFiStation()
    {
        // Unregister event handlers only if they were previously registered.
        if (wifiEventInstance_ != nullptr)
        {
            esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, wifiEventInstance_);
        }

        if (ipEventInstance_ != nullptr)
        {
            esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, ipEventInstance_);
        }

        // Delete the event group if it was created.
        if (wifiEventGroup_ != nullptr)
        {
            vEventGroupDelete(wifiEventGroup_);
            wifiEventGroup_ = nullptr;
        }
    }

    bool WiFiStation::connect()
    {
        ESP_LOGI(TAG, "Entering Wi-Fi station connect flow");

        createEventGroup();
        initNetif();
        createDefaultEventLoop();
        createDefaultStaInterface();
        initDriver();
        registerEventHandlers();
        configureStation();
        start();

        return waitForConnection();
    }

    bool WiFiStation::isConnected() const
    {
        return connected_;
    }

    void WiFiStation::eventHandlerThunk(void *arg,
                                        esp_event_base_t eventBase,
                                        int32_t eventId,
                                        void *eventData)
    {
        auto *self = static_cast<WiFiStation *>(arg);
        if (self != nullptr)
        {
            self->handleEvent(eventBase, eventId, eventData);
        }
    }

    void WiFiStation::handleEvent(esp_event_base_t eventBase,
                                  int32_t eventId,
                                  void *eventData)
    {
        if (eventBase == WIFI_EVENT && eventId == WIFI_EVENT_STA_START)
        {
            ESP_LOGI(TAG, "WIFI_EVENT_STA_START received, requesting connection");
            ESP_ERROR_CHECK(esp_wifi_connect());
        }
        else if (eventBase == WIFI_EVENT && eventId == WIFI_EVENT_STA_DISCONNECTED)
        {
            connected_ = false;

            if (retryCount_ < maxRetries_)
            {
                ++retryCount_;
                ESP_LOGW(TAG, "Disconnected. Retry %d/%d", retryCount_, maxRetries_);
                ESP_ERROR_CHECK(esp_wifi_connect());
            }
            else
            {
                ESP_LOGE(TAG, "Connection failed after %d retries", maxRetries_);
                xEventGroupSetBits(wifiEventGroup_, WIFI_FAIL_BIT);
            }
        }
        else if (eventBase == IP_EVENT && eventId == IP_EVENT_STA_GOT_IP)
        {
            auto *event = static_cast<ip_event_got_ip_t *>(eventData);

            ESP_LOGI(TAG, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));

            retryCount_ = 0;
            connected_ = true;
            xEventGroupSetBits(wifiEventGroup_, WIFI_CONNECTED_BIT);
        }
    }

    void WiFiStation::createEventGroup()
    {
        wifiEventGroup_ = xEventGroupCreate();
        if (wifiEventGroup_ == nullptr)
        {
            ESP_LOGE(TAG, "Failed to create Wi-Fi event group");
            abort();
        }

        ESP_LOGI(TAG, "Wi-Fi event group created");
    }

    void WiFiStation::initNetif()
    {
        ESP_ERROR_CHECK(esp_netif_init());
        ESP_LOGI(TAG, "esp_netif initialized");
    }

    void WiFiStation::createDefaultEventLoop()
    {
        esp_err_t err = esp_event_loop_create_default();

        // If the default loop already exists, we do not fail.
        if (err != ESP_OK && err != ESP_ERR_INVALID_STATE)
        {
            ESP_ERROR_CHECK(err);
        }

        ESP_LOGI(TAG, "Default event loop ready");
    }

    void WiFiStation::createDefaultStaInterface()
    {
        esp_netif_t *staNetif = esp_netif_create_default_wifi_sta();
        if (staNetif == nullptr)
        {
            ESP_LOGE(TAG, "Failed to create default Wi-Fi STA interface");
            abort();
        }

        ESP_LOGI(TAG, "Default Wi-Fi STA interface created");
    }

    void WiFiStation::initDriver()
    {
        wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
        ESP_ERROR_CHECK(esp_wifi_init(&cfg));
        ESP_LOGI(TAG, "Wi-Fi driver initialized");
    }

    void WiFiStation::registerEventHandlers()
    {
        ESP_ERROR_CHECK(esp_event_handler_instance_register(
            WIFI_EVENT,
            ESP_EVENT_ANY_ID,
            &WiFiStation::eventHandlerThunk,
            this,
            &wifiEventInstance_));

        ESP_ERROR_CHECK(esp_event_handler_instance_register(
            IP_EVENT,
            IP_EVENT_STA_GOT_IP,
            &WiFiStation::eventHandlerThunk,
            this,
            &ipEventInstance_));

        ESP_LOGI(TAG, "Wi-Fi and IP event handlers registered");
    }

    void WiFiStation::configureStation()
    {
        wifi_config_t wifiConfig;
        std::memset(&wifiConfig, 0, sizeof(wifiConfig));

        std::memcpy(wifiConfig.sta.ssid, ssid_, std::strlen(ssid_));
        std::memcpy(wifiConfig.sta.password, password_, std::strlen(password_));

        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
        ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifiConfig));

        ESP_LOGI(TAG, "Wi-Fi station configured for SSID: %s", ssid_);
    }

    void WiFiStation::start()
    {
        ESP_ERROR_CHECK(esp_wifi_start());
        ESP_LOGI(TAG, "Wi-Fi start requested");
    }

    bool WiFiStation::waitForConnection()
    {
        ESP_LOGI(TAG, "Waiting for connection result");

        EventBits_t bits = xEventGroupWaitBits(
            wifiEventGroup_,
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY);

        if ((bits & WIFI_CONNECTED_BIT) != 0)
        {
            ESP_LOGI(TAG, "Connected successfully to SSID: %s", ssid_);
            return true;
        }

        if ((bits & WIFI_FAIL_BIT) != 0)
        {
            ESP_LOGE(TAG, "Failed to connect to SSID: %s", ssid_);
            return false;
        }

        ESP_LOGE(TAG, "Unexpected event group state");
        return false;
    }

} // namespace radar