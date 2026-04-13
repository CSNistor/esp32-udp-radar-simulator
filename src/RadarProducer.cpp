#include "RadarProducer.hpp"
#include "esp_log.h"
#include "freertos/task.h"
#include "esp_timer.h"

namespace radar
{
    static const char *TAG = "RadarProducer";

    bool RadarProducer::startTask(const char *taskName, uint32_t stackSize, UBaseType_t priority)
    {
        BaseType_t result = xTaskCreate(
            &RadarProducer::taskEntry,
            taskName,
            stackSize,
            this,
            priority,
            nullptr);
        if (result != pdPASS)
        {
            ESP_LOGE(TAG, "Failed to create RadarProducer task");
            return false;
        }

        ESP_LOGI(TAG, "RadarProducer task created");
        return true;
    }

    void RadarProducer::taskEntry(void *arg)
    {
        auto *self = static_cast<RadarProducer *>(arg);

        if (self == nullptr)
        {
            ESP_LOGE(TAG, "RadarProducer task received null instance");
            vTaskDelete(nullptr);
            return;
        }

        self->run();

        // In case run() ever returns, cleanly delete the current task.
        vTaskDelete(nullptr);
    }

    void RadarProducer::updateSample(radar::RadarSample &sample)
    {
        sample.sequenceCounter = sequenceCounter_;
        int64_t microSeconds = esp_timer_get_time();
        sample.timestampMs = static_cast<uint32_t>(microSeconds / 1000); // convert to milliseconds
        sample.angleDeg = currentAngle_;
        sample.distanceM = currentDistance_;
        sample.speedMps = 1.5f + currentAngle_ * 0.01f;
        sample.objectId = 1;
        sequenceCounter_++;
        currentAngle_ += 0.1f;
        currentDistance_ += 0.2f;
        if (currentAngle_ >= 180.0f)
        {
            currentAngle_ = 0.0f;
        }
        if (currentDistance_ >= 100.0f)
        {
            currentDistance_ = 0.0f;
        }
    }

    void RadarProducer::run()
    {
        radar::RadarSample sample{};
        while (true)
        {
            updateSample(sample);
            ESP_LOGI(TAG,
                     "sample seq=%lu ts=%lu angle=%.2f dist=%.2f speed=%.2f obj=%u",
                     static_cast<unsigned long>(sample.sequenceCounter),
                     static_cast<unsigned long>(sample.timestampMs),
                     sample.angleDeg,
                     sample.distanceM,
                     sample.speedMps,
                     sample.objectId);
            BaseType_t result = xQueueSend(
                sampleQueue_,
                &sample,
                pdMS_TO_TICKS(2000));
            if (result != pdPASS)
            {
                ESP_LOGE(TAG, "Failed to post radar sample to queue");
                vTaskDelay(pdMS_TO_TICKS(100));
                continue;
            }
            ESP_LOGI(TAG, "Radar sample posted to queue");
            vTaskDelay(pdMS_TO_TICKS(2000));
        }
    }
}