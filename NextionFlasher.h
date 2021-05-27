#ifndef NextionFlasher_h
#define NextionFlasher_h

#include <string>

#include "FileReader.h"
#include "UARTLink.h"

#define DEFAULT_FLASHING_RATE 921600
#define DEFAULT_CONNECT_BAUDRATE 9600
#define OK_RESPONSE_CODE 0x05
#define PACKET_SIZE 4096
#define DEFAULT_RESPONSE_TIMEOUT 500
#define INITAL_RESPONE_SIZE 2
#define INITAL_RESPONE_HEADER "comok"
#define DEVICE_INFO_PACKET_SIZE 7

#define CONNECT_COMMAND "connect"

class NextionFlasher {
   public:
    NextionFlasher(std::string port);

    void connect();

    void readChip();

    void flashFile(FileReader::file_struct file);
    void flashData(uint8_t *data, uint32_t size);
    void setFlashingBaudRate(int baudRate);

   private:
    UARTLink *uart;

    UARTLink::buffer_struct *buffer;

    void flashCommand(uint32_t start_address, uint8_t *buffer, uint16_t length);
    void eraseCommand();

   private:
    static const int availableBaudRates[];

    uint8_t is_port_open = false;
    uint8_t is_connection_open = false;

    int flashingBaudRate = DEFAULT_FLASHING_RATE;
    int connectBaudRate = DEFAULT_CONNECT_BAUDRATE;

    void writeCommand(std::string command);
    void writeAddress(uint32_t address);
    void sendPackets(uint8_t *data, uint32_t size);

    void decodeDeviceInfoPacket(std::string packet);

    uint8_t checkConnectResponse();
};

#endif