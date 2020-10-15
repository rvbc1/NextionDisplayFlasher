#include "NextionFlasher.h"

#include <chrono>
#include <iostream>
#include <sstream>
#include <vector>

#define DEBUG

void printInfo(std::string info) {
    std::cout << std::dec << info << std::endl;
}

NextionFlasher::NextionFlasher(std::string port) {
    uart = new UARTLink(port);
    is_port_open = uart->openPort();

    buffer = uart->getBuff();

    connect();
}

void NextionFlasher::connect() {
    if (is_port_open) {
        // uart->addDataToBufferTX(START_CODE);
        //    uart->writeData();
        //    is_connection_open = checkResponse(ACK_AT_BEGIN);
        writeCommand("");
        writeCommand("connect");

        checkConnectResponse();

#if defined(__linux__) || defined(__FreeBSD__) || (__Windows__)
        if (is_connection_open == false) {
            printInfo("Cannot start comunication with uC");
            printInfo("Check wiring & set bootloader at chip");
            exit(1);
        }
#endif
    }
}

void NextionFlasher::checkConnectResponse() {
    buffer->size = uart->waitForResponse(500);

    std::string respone;
    for (int i = 0; i < buffer->size; i++) {
        respone += buffer->data[i];
    }
    //  std::cout << respone;

    std::vector<std::string> strings;
    std::istringstream f(respone);
    std::string s;
    while (getline(f, s, ' ')) {
        strings.push_back(s);
    }

    if ((strings.size() == 2) && (strings[0] == "comok")) {
        std::cout << "Connection ok!\n";
        std::vector<std::string> deviceInfo;
        std::istringstream f2(strings[1]);
        std::string s2;
        while (getline(f2, s2, ',')) {
            deviceInfo.push_back(s2);
        }
        if (deviceInfo.size() == 7) {
            std::cout << "Nextion touch: " << deviceInfo[0] << std::endl;
            std::cout << "Nextion address: " << deviceInfo[1] << std::endl;
            std::cout << "Nextion model: " << deviceInfo[2] << std::endl;
            std::cout << "Nextion firmware: " << deviceInfo[3] << std::endl;
            std::cout << "Nextion MCU: " << deviceInfo[4] << std::endl;
            std::cout << "Nextion serial: " << deviceInfo[5] << std::endl;
            std::cout << "Nextion flash size: " << deviceInfo[6] << std::endl;
        }
    }
}

void NextionFlasher::writeCommand(std::string command) {
    if (is_port_open) {
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

    writeCommand("");
    writeCommand(flash_cmd);

    uart->changeBaudRate(flashingBaudRate);

    buffer->size = uart->waitForFirstResponse(1000);

    printInfo("size " + std::to_string(buffer->size) + " respone " + std::to_string(buffer->data[0]));

    if ((buffer->size == 1) && (buffer->data[0] == 0x05)) {
        printInfo("Staring flashing " + std::to_string(size) + " bytes on speed " + std::to_string(flashingBaudRate));

        sendPackets(data, size);
    } else {
        printInfo("Nextion dont response for flashing command!");
    }
}

void NextionFlasher::sendPackets(uint8_t *data, uint32_t size) {
    uint32_t packets_amount = (size / PACKET_SIZE);
    for (int i = 0; i <= packets_amount; i++) {
        for (int j = 0; j < PACKET_SIZE; j++) {
            int byte_index = j + PACKET_SIZE * i;
            if (byte_index == size) {
                goto out;
            }
            uart->addDataToBufferTX(data[byte_index]);
        }
        uart->writeData();
        buffer->size = uart->waitForFirstResponse(500);
        uart->writing_buffer.size = 0;

        if ((buffer->size == 1) && (buffer->data[0] == 0x05)) {
            printInfo("Packet " + std::to_string(i) + "/" + std::to_string(packets_amount) + " sent");
        } else {
            printInfo("No confirming byte in response for " + std::to_string(i) + "/" + std::to_string(packets_amount) + " packet");
        }
    }
out:
    uart->writeData();
    buffer->size = uart->waitForFirstResponse(500);
    if ((buffer->size == 1) && (buffer->data[0] == 0x05)) {
        printInfo("Packet " + std::to_string(packets_amount) + "/" + std::to_string(packets_amount) + " sent");
    } else {
        printInfo("No confirming byte in response for " + std::to_string(packets_amount) + "/" + std::to_string(packets_amount) + " packet");
    }
}
