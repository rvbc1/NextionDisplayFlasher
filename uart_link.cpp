#include "uart_link.h"



#include <stdlib.h>
#include <stdio.h>

#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

#include "rs232.h"


#include "string"

uint32_t crc32(uint32_t crc, const char *buf, size_t len)
{
    static uint32_t table[256];
    static int have_table = 0;
    uint32_t rem;
    uint8_t octet;
    int i, j;
    const char *p, *q;

    /* This check is not thread safe; there is no mutex. */
    if (have_table == 0) {
        /* Calculate CRC table. */
        for (i = 0; i < 256; i++) {
            rem = i;  /* remainder from polynomial division */
            for (j = 0; j < 8; j++) {
                if (rem & 1) {
                    rem >>= 1;
                    rem ^= 0xedb88320;
                } else
                    rem >>= 1;
            }
            table[i] = rem;
        }
        have_table = 1;
    }

    crc = ~crc;
    q = buf + len;
    for (p = buf; p < q; p++) {
        octet = *p;  /* Cast to unsigned octet. */
        crc = (crc >> 8) ^ table[(crc & 0xff) ^ octet];
    }
    return ~crc;
}


UART_Link::UART_Link(uint8_t com_port, int baud_rate, uint16_t buffer_size)
{
    this->com_port = com_port;        /* /dev/ttyS0 (COM1 on windows) */
    this->baud_rate = baud_rate;       /* 9600 baud */
    this->buffer_size = buffer_size;

    char mode[]={'8','E','1',0};


    buffer = new uint8_t [buffer_size];


    if(RS232_OpenComport(com_port, baud_rate, mode, 0))
    {
        printf("Can not open comport\n");
    }

    frameTXsize = sizeof(FrameTX);
    frameRXsize = sizeof(FrameRX);
    dataTX.bytes = new uint8_t [frameTXsize];
    dataRX.bytes = new uint8_t [frameRXsize];

    //dataTX.frame->frame_size = frameTXsize;
//	dataRX.frame->frame_size = frameRXsize;

    dataTX.frame->start_code = 0x40;
    dataTX.frame->end_code = 0x80;
    dataTX.frame->values.ele_angle = 15;
}



uint8_t UART_Link::recieve(){
    reciev_bytes = RS232_PollComport(com_port, buffer, buffer_size);

    if(find_packet()){
        //crc16(dataRX.bytes, 6);
        return true;
    }
    /*
     std::string s = "";
    if(reciev_bytes > 0){
         for(int i = 0; i < reciev_bytes; i++){
             s += std::to_string(buffer[i]);
             s += " ";
         }
         qDebug() << s.c_str();
    }
    */
   return false;
}

void UART_Link::sendData(){
    RS232_SendBuf(com_port, dataTX.bytes, frameTXsize);
}

UART_Link::ValuesRX UART_Link::getValuesRX(){
    return dataRX.frame->values;
}

UART_Link::ValuesTX *UART_Link::getValuesTXBuff(){
    return &dataTX.frame->values;
}

uint8_t UART_Link::find_packet(){
    if(reciev_bytes > 0){
        for(int i = 0; i < reciev_bytes; i++){
            if(buffer[i] == START_CODE && buffer[i + (frameRXsize - 1)] == END_CODE){
                for(int k = 0; k < frameRXsize; k++){
                    dataRX.bytes[k] = buffer[i + k];
                }
                return true;
            }
        }
    }
    return false;
}
