#ifndef ASYNC_PACKET_BUFFER_HPP
#define ASYNC_PACKET_BUFFER_HPP

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>

namespace AsyncPacketBuffer {

// Opaque handle kept internal; global singleton for simplicity.
struct Handle;

/**
 * @brief Start the async TX buffer worker.
 * @param stream   Stream to write to (e.g., Serial)
 * @param taskCore Core to pin worker to (ESP32: 0 or 1)
 * @param taskPrio FreeRTOS priority (1 is fine)
 * @param queueLen Number of packets buffered
 * @return Handle* or nullptr on failure
 */
Handle* begin(Stream& stream, uint8_t taskCore = 1, UBaseType_t taskPrio = 1, uint16_t queueLen = 16);

/**
 * @brief Enqueue a complete payload (without length prefix).
 * Adds a 2-byte big-endian length prefix on send.
 */
bool send(const uint8_t* data, uint16_t len);

/** @brief Is the async buffer active? */
bool isActive();

/** @brief Optional stop (not typically used on Arduino lifecycle). */
void end();

} // namespace AsyncPacketBuffer

#endif // ASYNC_PACKET_BUFFER_HPP
