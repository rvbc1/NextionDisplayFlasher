#ifndef NextionFlasher_h
#define NextionFlasher_h

#include <string>

#include "FileReader.h"
#include "UARTLink.h"

#define DEFAULT_FLASHING_RATE 921600
#define PACKET_SIZE 4096

class NextionFlasher {
   public:
    NextionFlasher(std::string port);

    void connect();

    void readChip();

    void flashFile(FileReader::file_struct file);
    void flashData(uint8_t *data, uint32_t size);

   private:
    UARTLink *uart;

    UARTLink::buffer_struct *buffer;

    void flashCommand(uint32_t start_address, uint8_t *buffer, uint16_t length);
    void eraseCommand();

   private:
    uint8_t is_port_open = false;
    uint8_t is_connection_open = false;

    int flashingBaudRate = DEFAULT_FLASHING_RATE;


    void writeCommand(std::string command);
    void writeAddress(uint32_t address);
    void sendPackets(uint8_t *data, uint32_t size);

    void checkConnectResponse();
};

#endif