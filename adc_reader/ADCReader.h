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

class CircularBuffer {
private:
    std::array<ADCMeasurement, 500> buffer;
    size_t head = 0;
    size_t tail = 0;
    size_t count = 0;

public:
    void push(const ADCMeasurement& m);
    bool popOldest(ADCMeasurement& out);
    bool popNewest(ADCMeasurement& out);
    void clear();
    size_t size() const { return count; }
    bool empty() const { return count == 0; }
    bool full() const { return count == buffer.size(); }
    
    // Für Kompatibilität mit bestehendem Code
    const ADCMeasurement& operator[](size_t index) const;
    size_t getHead() const { return head; }
    size_t capacity() const { return buffer.size(); }
};

class ADCReader {
public:
    ADCReader(ADC78H89* adc, TimerCallback* timer);
    void begin(const std::vector<uint8_t>& channels, float totalFrequency);
    CircularBuffer& getChannelBuffer(uint8_t channel);
    const CircularBuffer& getChannelBuffer(uint8_t channel) const;

private:
    ADC78H89* _adc;
    TimerCallback* _timer;
    std::vector<uint8_t> _channels;
    size_t _currentChannelIndex = 0;
    std::map<uint8_t, CircularBuffer> _buffers;

    static void samplingCallback(void* context);
    void sample();
};
