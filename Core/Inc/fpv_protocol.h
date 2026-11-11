#ifndef FPV_PROTOCOL_H
#define FPV_PROTOCOL_H

#include <stdint.h>
#include <stdbool.h>

#define FPV_MAGIC_1 0x46
#define FPV_MAGIC_2 0x50

typedef struct __attribute__((packed))
{
    uint8_t magic1;
    uint8_t magic2;
    uint8_t receiver_id;
    uint8_t detector_id;
    uint8_t threat;
    uint16_t rms;
    uint16_t peak;
    uint16_t fpv_score;
    uint16_t noise_floor;
    uint16_t seq;
    uint8_t crc;
} fpv_packet_t;

#define FPV_PACKET_SIZE sizeof(fpv_packet_t)

uint8_t fpv_crc8(const uint8_t *data, uint8_t len);

void fpv_packet_make(
    fpv_packet_t *p,
    uint8_t receiver_id,
    uint8_t detector_id,
    uint8_t threat,
    uint16_t rms,
    uint16_t peak,
    uint16_t fpv_score,
    uint16_t noise_floor,
    uint16_t seq
);

bool fpv_packet_check(const fpv_packet_t *p);

#endif
