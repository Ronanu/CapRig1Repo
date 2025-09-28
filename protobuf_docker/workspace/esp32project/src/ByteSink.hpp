#ifndef BYTESINK_HPP
#define BYTESINK_HPP

#include <stdint.h>
#include <stddef.h>

typedef bool (*ByteSinkFn)(const uint8_t* data, uint16_t len);
extern ByteSinkFn g_proto_send_bytes;

#endif // BYTESINK_HPP
