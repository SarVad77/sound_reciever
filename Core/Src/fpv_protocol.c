#include "fpv_protocol.h"

uint8_t fpv_crc8(const uint8_t *data, uint8_t len)
{
    uint8_t crc = 0xA5;

    for (uint8_t i = 0; i < len; i++)
    {
        crc ^= data[i];

        for (uint8_t b = 0; b < 8; b++)
        {
            if (crc & 0x80)
                crc = (crc << 1) ^ 0x31;
            else
                crc <<= 1;
        }
    }

    return crc;
}

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
)
{
    p->magic1 = FPV_MAGIC_1;
    p->magic2 = FPV_MAGIC_2;

    p->receiver_id = receiver_id & 0x07;
    p->detector_id = detector_id & 0x1F;
    p->threat = threat ? 1 : 0;

    p->rms = rms;
    p->peak = peak;
    p->fpv_score = fpv_score;
    p->noise_floor = noise_floor;
    p->seq = seq;

    p->crc = fpv_crc8((uint8_t *)p, FPV_PACKET_SIZE - 1);
}

bool fpv_packet_check(const fpv_packet_t *p)
{
    if (p->magic1 != FPV_MAGIC_1) return false;
    if (p->magic2 != FPV_MAGIC_2) return false;

    return p->crc == fpv_crc8((const uint8_t *)p, FPV_PACKET_SIZE - 1);
}
