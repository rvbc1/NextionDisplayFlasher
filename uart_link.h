#ifndef UART_LINK_H
#define UART_LINK_H

//#include <QSerialPort>
//#include <QSerialPortInfo>
#include "stdint.h"

#define START_CODE 0x40
#define END_CODE 0x80

class UART_Link
{
public:
    UART_Link(uint8_t com_port, int baud_rate, uint16_t buffer_size);

    uint8_t recieve();

    void sendData();


    struct ValuesTX{
        uint16_t ele_angle;

        uint16_t SERVO_1;

        uint16_t SERVO_2;

        uint16_t SERVO_3;

        uint16_t SERVO_4;

    } __attribute__ ((__packed__));

    struct ValuesRX{
        uint16_t ele_angle;

        uint16_t enc1_raw;

        uint16_t enc1_angle;

        uint16_t enc2_raw;

        uint16_t enc2_angle;

        uint16_t enc3_raw;

        uint16_t enc3_angle;

        uint16_t enc4_raw;

        uint16_t enc4_angle;

        uint16_t batt;
    } __attribute__ ((__packed__));

    ValuesRX getValuesRX();
    ValuesTX *getValuesTXBuff();

private:
    int baud_rate;
    uint8_t com_port;
    int reciev_bytes;
    uint8_t* buffer;
    uint16_t buffer_size;

    uint16_t frameTXsize;
    uint16_t frameRXsize;

    struct FrameTX{

        uint8_t start_code;
//        uint16_t frame_size;
        ValuesTX values;
        uint8_t end_code;
   //     uint16_t crc;

    } __attribute__ ((__packed__));

    struct FrameRX{
        uint8_t start_code;
       // uint16_t frame_size;
        ValuesRX values;
        uint8_t end_code;
      //  uint16_t crc;
    } __attribute__ ((__packed__));

    union DataTX_u{
        FrameTX* frame;
        uint8_t* bytes;
    } dataTX;

    union DataRX_u{
        FrameRX *frame;
        uint8_t *bytes;
    } dataRX;


    uint8_t find_packet();
};

#endif // UART_LINK_H
