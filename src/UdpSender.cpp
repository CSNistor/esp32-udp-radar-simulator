#include "UdpSender.hpp"

#include <cstring>
#include <cstdio>

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "lwip/sockets.h"
#include "lwip/inet.h"
#include "lwip/netdb.h"

namespace radar
{
    static const char *TAG = "UdpSender";

    UdpSender::UdpSender(const char *destIp, uint16_t destPort, QueueHandle_t sampleQueue)
        : destIp_(destIp),
          destPort_(destPort),
          sampleQueue_(sampleQueue),
          sock_(-1)
    {
        std::memset(&destAddr_, 0, sizeof(destAddr_));
    }

    UdpSender::~UdpSender()
    {
        if (sock_ != -1)
        {
            ESP_LOGI(TAG, "Closing UDP socket");
            close(sock_);
            sock_ = -1;
        }
    }

    bool UdpSender::init()
    {
        ESP_LOGI(TAG, "Initializing UDP sender");

        if (!createSocket())
        {
            return false;
        }

        if (!configureDestination())
        {
            return false;
        }

        ESP_LOGI(TAG, "UDP sender initialized (dest=%s:%u)", destIp_, destPort_);
        return true;
    }

    bool UdpSender::createSocket()
    {
        sock_ = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);

        if (sock_ < 0)
        {
            ESP_LOGE(TAG, "Failed to create socket: errno=%d", errno);
            return false;
        }

        ESP_LOGI(TAG, "Socket created");
        return true;
    }

    bool UdpSender::configureDestination()
    {
        destAddr_.sin_family = AF_INET;
        destAddr_.sin_port = htons(destPort_);

        int result = inet_pton(AF_INET, destIp_, &destAddr_.sin_addr);

        if (result <= 0)
        {
            ESP_LOGE(TAG, "Invalid destination IP: %s", destIp_);
            return false;
        }

        ESP_LOGI(TAG, "Destination configured");
        return true;
    }

    bool UdpSender::send(const char *data, size_t length)
    {
        if (sock_ < 0)
        {
            ESP_LOGE(TAG, "Socket not initialized");
            return false;
        }

        int sent = sendto(
            sock_,
            data,
            length,
            0,
            reinterpret_cast<struct sockaddr *>(&destAddr_),
            sizeof(destAddr_));

        if (sent < 0)
        {
            ESP_LOGE(TAG, "Send failed: errno=%d", errno);
            return false;
        }

        ESP_LOGI(TAG, "Sent %d bytes", sent);
        return true;
    }

    bool UdpSender::startTask(const char *taskName, uint32_t stackSize, UBaseType_t priority)
    {
        BaseType_t result = xTaskCreate(
            &UdpSender::taskEntry, // static wrapper
            taskName,
            stackSize,
            this, // pass current object as generic context
            priority,
            nullptr);

        if (result != pdPASS)
        {
            ESP_LOGE(TAG, "Failed to create UDP sender task");
            return false;
        }

        ESP_LOGI(TAG, "UDP sender task created");
        return true;
    }

    void UdpSender::taskEntry(void *arg)
    {
        auto *self = static_cast<UdpSender *>(arg);

        if (self == nullptr)
        {
            ESP_LOGE(TAG, "UdpSender task received null instance");
            vTaskDelete(nullptr);
            return;
        }

        self->run();

        // In case run() ever returns, cleanly delete the current task.
        vTaskDelete(nullptr);
    }

    void UdpSender::run()
    {
        ESP_LOGI(TAG, "Starting UDP sender task loop");
        RadarSample sample{};
        while (true)
        {
            BaseType_t result = xQueueReceive(
                sampleQueue_,
                &sample,
                portMAX_DELAY);
            if (result != pdPASS)
            {
                ESP_LOGE(TAG, "Failed to read radar sample from queue");
                vTaskDelay(pdMS_TO_TICKS(100));
                continue;
            }
            ESP_LOGI(TAG,
                     "sample seq=%lu ts=%lu angle=%.2f dist=%.2f speed=%.2f obj=%u",
                     static_cast<unsigned long>(sample.sequenceCounter),
                     static_cast<unsigned long>(sample.timestampMs),
                     sample.angleDeg,
                     sample.distanceM,
                     sample.speedMps,
                     sample.objectId);
            if (!send(reinterpret_cast<const char *>(&sample), sizeof(sample)))
            {
                ESP_LOGE(TAG, "Failed to send radar sample over UDP");
            }
        }
    }

} // namespace radar