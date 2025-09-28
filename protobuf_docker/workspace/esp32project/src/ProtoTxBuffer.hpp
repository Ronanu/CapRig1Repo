#ifndef PROTO_TX_BUFFER_HPP
#define PROTO_TX_BUFFER_HPP

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>

bool proto_txbuffer_begin(Stream& serial,
                          uint8_t taskCore = 1,
                          UBaseType_t taskPrio = 1,
                          uint16_t queueLen = 16);

bool proto_txbuffer_enqueue(const uint8_t* data, uint16_t len);

bool proto_txbuffer_active();

#endif // PROTO_TX_BUFFER_HPP
