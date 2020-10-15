#include "NextionFlasher.h"

#include <chrono>
#include <iostream>
#include <sstream>
#include <vector>

//#define DEBUG

//ALL AVAILABLE BOUD_RATES FOR NEXTION DISPLAY
//2400, 4800, 9600, 19200,31250 38400, 57600, 115200, 230400, 250000, 256000, 512000, 921600

#if defined(__linux__) || defined(__FreeBSD__)
//ALL AVAILABLE BOUD_RATES_FOR IN RS232_LIB IN LINUX
const int NextionFlasher::availableBaudRates[] = {2400, 4800, 9600, 19200, 38400, 57600, 115200, 230400, 921600};
#else
//ALL AVAILABLE BOUD_RATES_FOR IN RS232_LIB IN WINDOWS
const int NextionFlasher::availableBaudRates[] = {2400, 4800, 9600, 19200, 38400, 57600, 115200, 230400, 256000, 921600};
#endif

void printInfo(std::string info, uint8_t writeNewLine = true) {
    if (writeNewLine) {
        std::cout << std::dec << info << std::endl;
    } else {
        std::cout << std::dec << info;
    }
}

std::string BoolToString(uint8_t b) {
    return b ? "true" : "false";
}

NextionFlasher::NextionFlasher(std::string port) {
    uart = new UARTLink(port);
    is_port_open = uart->openPort();

    buffer = uart->getBuff();
}

void NextionFlasher::connect() {
    if (is_port_open) {
        printInfo("Connecting", false);
        for (uint32_t i = 0; i < (sizeof(availableBaudRates) / sizeof(*availableBaudRates)); i++) {
            uart->changeBaudRate(availableBaudRates[i]);
            writeCommand("connect");
            if (checkConnectResponse())
                break;
        }
        
        //checkConnectResponse();

#if defined(__linux__) || defined(__FreeBSD__) || (__Windows__)
        if (is_connection_open == false) {
            printInfo("Cannot start comunication with uC");
            printInfo("Check wiring & set bootloader at chip");
            exit(1);
        }
#endif
    }
}

uint8_t NextionFlasher::checkConnectResponse() {
    buffer->size = uart->waitForResponse(DEFAULT_RESPONSE_TIMEOUT);

    std::string rawRespone;
    for (int i = 0; i < buffer->size; i++) {
        rawRespone += buffer->data[i];
    }

    std::vector<std::string> responseParts;
    std::istringstream stream(rawRespone);
    std::string tempString;
    while (getline(stream, tempString, ' ')) {
        responseParts.push_back(tempString);
    }

    if ((responseParts.size() == INITAL_RESPONE_SIZE) && (responseParts[0] == "comok")) {
        printInfo("\rConnected to device:");
        decodeDeviceInfoPacket(responseParts[1]);
        return true;
    }
    return false;
}

void NextionFlasher::decodeDeviceInfoPacket(std::string packet) {
    std::vector<std::string> deviceInfo;
    std::istringstream stream(packet);
    std::string tempString;
    while (getline(stream, tempString, ',')) {
        deviceInfo.push_back(tempString);
    }
    if (deviceInfo.size() == DEVICE_INFO_PACKET_SIZE) {
        printInfo("\tModel: " + deviceInfo[2]);
        printInfo("\tAddress: " + deviceInfo[1]);
        printInfo("\tMCU code: " + deviceInfo[4]);
        printInfo("\tFlash size: " + deviceInfo[6]);
        printInfo("\tTouch ability: " + BoolToString(std::stoi(deviceInfo[0])));
        printInfo("\tFirmware version: " + deviceInfo[3]);
        printInfo("\tSerial: " + deviceInfo[5] + "\n");
    }
}

void NextionFlasher::writeCommand(std::string command) {
    if (is_port_open) {
        uart->addDataToBufferTX(0xFF);
        uart->addDataToBufferTX(0xFF);
        uart->addDataToBufferTX(0xFF);
        uart->addDataToBufferTX(command);
        uart->addDataToBufferTX(0xFF);
        uart->addDataToBufferTX(0xFF);
        uart->addDataToBufferTX(0xFF);
        uart->writeData();
    }
}

void NextionFlasher::flashFile(FileReader::file_struct file) {
    flashData(file.data, file.size);
}

void NextionFlasher::flashData(uint8_t *data, uint32_t size) {
    std::string flash_cmd = "whmi-wri " + std::to_string(size) + "," + std::to_string(flashingBaudRate) + ",0";

#ifdef DEBUG
    printInfo("Flashing cmd :\"" + flash_cmd + "\"");
#endif
    writeCommand(flash_cmd);

    uart->changeBaudRate(flashingBaudRate);

    buffer->size = uart->waitForFirstResponse(DEFAULT_RESPONSE_TIMEOUT);

    if ((buffer->size == 1) && (buffer->data[0] == OK_RESPONSE_CODE)) {
        printInfo("Staring flashing " + std::to_string(size) + " bytes on speed " + std::to_string(flashingBaudRate));

        sendPackets(data, size);
    } else {
        printInfo("Nextion dont response for flashing command");
    }
}

void NextionFlasher::sendPackets(uint8_t *data, uint32_t size) {
    uint32_t packets_amount = (size / PACKET_SIZE);
    for (uint32_t i = 0; i <= packets_amount; i++) {
        for (uint32_t j = 0; j < PACKET_SIZE; j++) {
            uint32_t byte_index = j + PACKET_SIZE * i;
            if (byte_index == size) {
                goto out;
            }
            uart->addDataToBufferTX(data[byte_index]);
        }
        uart->writeData();
        buffer->size = uart->waitForFirstResponse(DEFAULT_RESPONSE_TIMEOUT);
        uart->writing_buffer.size = 0;

        if ((buffer->size == 1) && (buffer->data[0] == OK_RESPONSE_CODE)) {
            printInfo("\rPacket " + std::to_string(i) + "/" + std::to_string(packets_amount) + " sent", false);
        } else {
            printInfo("No confirmation code in response for " + std::to_string(i) + "/" + std::to_string(packets_amount) + " packet");
        }
    }
out:
    uart->writeData();
    buffer->size = uart->waitForFirstResponse(DEFAULT_RESPONSE_TIMEOUT);
    if ((buffer->size == 1) && (buffer->data[0] == OK_RESPONSE_CODE)) {
        printInfo("\rPacket " + std::to_string(packets_amount) + "/" + std::to_string(packets_amount) + " sent");
    } else {
        printInfo("No confirming byte in response for " + std::to_string(packets_amount) + "/" + std::to_string(packets_amount) + " packet");
    }
}


void NextionFlasher::setFlashingBaudRate(int baudRate){
    this->flashingBaudRate = baudRate;
}
