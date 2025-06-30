#include "ADCReader.h"

ADCReader::ADCReader(ADC78H89* adc, TimerCallback* timer)
    : _adc(adc), _timer(timer) {}

void ADCReader::begin(const std::vector<uint8_t>& channels, float totalFrequency) {
    _channels = channels;
    _currentChannelIndex = 0;

    // Buffer für alle Kanäle initialisieren
    for (auto ch : channels) {
        _buffers[ch] = ChannelBuffer();
    }

    _timer->begin(totalFrequency);
    _timer->attachCallback(samplingCallback, this);
    _timer->start();
}

const ChannelBuffer& ADCReader::getChannelBuffer(uint8_t channel) const {
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
