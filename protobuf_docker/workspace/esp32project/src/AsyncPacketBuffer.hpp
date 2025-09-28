#ifndef ASYNC_PACKET_BUFFER_HPP
#define ASYNC_PACKET_BUFFER_HPP

#include <Arduino.h>

// FreeRTOS forward declarations
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>

namespace AsyncPacketBuffer {

// Opaque handle for the buffer
struct Handle;

/**
 * Initialize the asynchronous packet buffer.
 * A worker task will write packets to the given Stream (length-prefixed: big-endian 16-bit + payload).
 * Returns a Handle pointer on success, or nullptr on failure.
 */
Handle* begin(Stream& stream, uint8_t taskCore = 1, UBaseType_t taskPrio = 1, uint16_t queueLen = 16);

/**
 * Enqueue one complete packet (payload only). The worker adds the 2-byte length prefix.
 * If called before begin(), returns false.
 */
bool send(const uint8_t* data, uint16_t len);

/**
 * Whether the global buffer is active (begin() was called successfully).
 */
bool isActive();

/**
 * Optional: stop and free resources.
 */
void end();

} // namespace AsyncPacketBuffer

#endif // ASYNC_PACKET_BUFFER_HPP
