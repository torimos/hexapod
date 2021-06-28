#include "SerialProtocol.h"
#include "crc.h"
#define EMPTY_BYTE 0x55

SerialProtocol::SerialProtocol(Stream* stream)
{
    this->stream = stream;
    memset(frame_buf, EMPTY_BYTE, sizeof(frame_buf));
}

void SerialProtocol::write16(uint16_t header, void* data, uint16_t size)
{
    uint16_t checksum = get_CRC16(data, size);
    stream->write((uint8_t*)&header, sizeof(uint16_t));
    stream->write((uint8_t*)&size, sizeof(uint16_t));
    stream->write((uint8_t*)data, size);
    stream->write((uint8_t*)&checksum, sizeof(uint16_t));
}

void SerialProtocol::write32(uint32_t header, void* data, uint16_t size)
{
    uint32_t checksum = get_CRC32(data, size);
    stream->write((uint8_t*)&header, sizeof(uint32_t));
    stream->write((uint8_t*)&size, sizeof(uint16_t));
    stream->write((uint8_t*)data, size);
    stream->write((uint8_t*)&checksum, sizeof(uint32_t));
}

bool SerialProtocol::read16(uint16_t header, void* data, uint16_t size) {
    frame_buf_read();
    return parse_frame16(header, data, size);
}

bool SerialProtocol::parse_frame16(uint16_t header, void* result_data, uint16_t expected_size)
{
    if (frame_buf_len > 0)
    {
        int frame_start_offset = 0;
        bool header_found = false;
        while (frame_start_offset < (frame_buf_len - 4)) // offset should include at least 4 bytes of [header]+[data len]
        {
            uint16_t* hdr = (uint16_t*)&frame_buf[frame_start_offset];
            if (*hdr == header)
            {
                header_found = true;
                break;
            }
            frame_start_offset++;
        }
        if (header_found)
        {
            // align frame start with 0 start index in buffer
            move_frame(frame_start_offset);

            //process frame if buffer len is as expected + 6 bytes = [header](2)+[data len](2)+[crc](2)
            if (frame_buf_len >= (expected_size+6))
            {
                uint16_t data_size = *(uint16_t*)&frame_buf[2]; //skip 2 bytes of [header]
                if (data_size == expected_size)
                {
                    uint16_t expected_crc = get_CRC16(&frame_buf[4], data_size);  //skip 2 bytes of [data len]
                    uint16_t crc = *(uint16_t*)&frame_buf[4 + data_size];
                    if (crc == expected_crc)
                    {
                        memcpy(result_data, &frame_buf[4], data_size);
                        move_frame(6 + data_size);
                        return true;
                    }
                    // remove invalid frame [header]+[data len]
                    else
                    {
                        move_frame(4);
                    }
                }
                // remove invalid frame [header]
                else
                {
                    move_frame(2);
                }
            }
        }
    }
    return false;
}

uint16_t SerialProtocol::stream_read(uint8_t* buffer, uint16_t size)
{
    size_t avail = stream->available();
    if (size < avail) {
        avail = size;
    }
    size_t count = 0;
    while (count < avail) {
        *buffer++ = stream->read();
        count++;
    }
    stream->flush();
    return count;
}

uint16_t SerialProtocol::frame_buf_read()
{
    size_t rx_buf_len = stream_read(rx_buf, sizeof(rx_buf));
    if (rx_buf_len <= 0)
        return rx_buf_len;
    else
    {
        if ((frame_buf_len + rx_buf_len) > sizeof(frame_buf))
        {
            // data overflow
            frame_buf_len = 0;
        }
        memcpy(frame_buf + frame_buf_len, rx_buf, rx_buf_len);
        frame_buf_len += rx_buf_len;
        return rx_buf_len;
    }
    return 0;
}

void SerialProtocol::move_frame(uint16_t offset)
{
    if (offset <= 0) return;
    memmove(frame_buf, &frame_buf[offset], frame_buf_len - offset);
    frame_buf_len = frame_buf_len - offset;
    if ((sizeof(frame_buf) - frame_buf_len) >= 1)
        memset(frame_buf + frame_buf_len, EMPTY_BYTE, sizeof(frame_buf) - frame_buf_len);
}