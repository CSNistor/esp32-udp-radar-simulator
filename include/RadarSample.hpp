#ifndef RADARSAMPLE_HPP
#define RADARSAMPLE_HPP
#include <cstdint>

namespace radar
{
    struct RadarSample
    {
        uint32_t sequenceCounter;
        uint32_t timestampMs;
        float distanceM;
        float speedMps;
        float angleDeg;
        uint16_t objectId;
    };
} // namespace RadarSample

#endif