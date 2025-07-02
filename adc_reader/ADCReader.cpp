#include "ADCReader.h"

// CircularBuffer Implementation
void CircularBuffer::push(const ADCMeasurement& m) {
    buffer[head] = m;
    head = (head + 1) % buffer.size();
    
    if (count < buffer.size()) {
        count++;
    } else {
        // Buffer voll, tail mitbewegen
        tail = (tail + 1) % buffer.size();
    }
}

bool CircularBuffer::popOldest(ADCMeasurement& out) {
    if (count == 0) return false;
    
    out = buffer[tail];
    tail = (tail + 1) % buffer.size();
    count--;
    return true;
}

bool CircularBuffer::popNewest(ADCMeasurement& out) {
    if (count == 0) return false;
    
    head = (head + buffer.size() - 1) % buffer.size();
    out = buffer[head];
    count--;
    return true;
}

void CircularBuffer::clear() {
    count = 0;
    head = 0;
    tail = 0;
}

const ADCMeasurement& CircularBuffer::operator[](size_t index) const {
    // Für Kompatibilität: Index basiert auf tail
    return buffer[(tail + index) % buffer.size()];
}

// ADCReader Implementation

ADCReader::ADCReader(ADC78H89* adc, TimerCallback* timer)
    : _adc(adc), _timer(timer) {}

void ADCReader::begin(const std::vector<uint8_t>& channels, float totalFrequency) {
    _channels = channels;
    _currentChannelIndex = 0;

    // Buffer für alle Kanäle initialisieren
    for (auto ch : channels) {
        _buffers[ch] = CircularBuffer();
    }

    _timer->begin(totalFrequency);
    _timer->attachCallback(samplingCallback, this);
    _timer->start();
}

CircularBuffer& ADCReader::getChannelBuffer(uint8_t channel) {
    return _buffers.at(channel);
}

const CircularBuffer& ADCReader::getChannelBuffer(uint8_t channel) const {
    return _buffers.at(channel);
}

void ADCReader::samplingCallback(void* context) {
    if (context) {
        static_cast<ADCReader*>(context)->sample();
    }
}

void ADCReader::sample() {
    uint32_t ts = micros();
    uint8_t ch = _channels[_currentChannelIndex];
    uint16_t val = _adc->readChannel(ch);

    _buffers[ch].push({val, ts});

    _currentChannelIndex = (_currentChannelIndex + 1) % _channels.size();
}
