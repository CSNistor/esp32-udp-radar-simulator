#ifndef RADAR_PRODUCER_HPP
#define RADAR_PRODUCER_HPP

#include <cstdint>

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#include "RadarSample.hpp"

namespace radar
{
    class RadarProducer
    {
    public:
        explicit RadarProducer(QueueHandle_t sampleQueue)
            : sampleQueue_(sampleQueue),
              sequenceCounter_(0),
              currentAngle_(0.0f),
              currentDistance_(1.0f)
        {
        }

        ~RadarProducer() = default;

        bool startTask(const char *taskName = "RadarProducerTask",
                       uint32_t stackSize = 4096,
                       UBaseType_t priority = 5);

    private:
        QueueHandle_t sampleQueue_;
        uint32_t sequenceCounter_;
        float currentAngle_;
        float currentDistance_;

    private:
        void run();
        static void taskEntry(void *arg);
        void updateSample(radar::RadarSample &sample);
    };

} // namespace radar

#endif // RADAR_PRODUCER_HPP