#pragma once
#include <Arduino.h>
#include <SPI.h>
#include <vector>
#include <map>
#include <array>
#include "ADC78H89.h"
#include "TimerCallback.h"

struct ADCMeasurement {
    uint16_t value;
    uint32_t timestamp;
};

struct ChannelBuffer {
    std::array<ADCMeasurement, 500> buffer;
    size_t head = 0;

    void push(const ADCMeasurement& m) {
        buffer[head] = m;
        head = (head + 1) % buffer.size();
    }
};

class ADCReader {
public:
    ADCReader(ADC78H89* adc, TimerCallback* timer);
    void begin(const std::vector<uint8_t>& channels, float totalFrequency);
    const ChannelBuffer& getChannelBuffer(uint8_t channel) const;

private:
    ADC78H89* _adc;
    TimerCallback* _timer;
    std::vector<uint8_t> _channels;
    size_t _currentChannelIndex = 0;
    std::map<uint8_t, ChannelBuffer> _buffers;

    static void samplingCallback(void* context);
    void sample();
};
