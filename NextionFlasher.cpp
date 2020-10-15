#include "NextionFlasher.h"

#include <chrono>
#include <iostream>
#include <sstream>
#include <vector>

void printInfo(std::string info) {
    std::cout << info << std::endl;
}

NextionFlasher::NextionFlasher(std::string port) {
    uart = new UARTLink(port);
    is_port_open = uart->openPort();
    //  uint8_t *buffer = uart->getBuff();
    buffer = uart->getBuff();
    //writing_buffer = &uart->writing_buffer;

    // openConnection();
    // getCommand();

    //readMemoryCommand(0x8000000, 0xFF);
    //eraseCommand();
    //readMemoryCommand(0x8000000, 0xFF);
    //uint8_t test;
    //flashCommand(0x8000000, &test, 0xFF);
    //readMemoryCommand(0x8000000, 0xFF);
}

void NextionFlasher::openConnection() {
    if (is_port_open) {
        // uart->addDataToBufferTX(START_CODE);
        //    uart->writeData();
        //    is_connection_open = checkResponse(ACK_AT_BEGIN);
        writeCommand("");
        writeCommand("connect");

        checkOpenConnectionResponse();

#if defined(__linux__) || defined(__FreeBSD__) || (__Windows__)
        if (is_connection_open == false) {
            printInfo("Cannot start comunication with uC");
            printInfo("Check wiring & set bootloader at chip");
            exit(1);
        }
#endif
    }
}

void NextionFlasher::checkOpenConnectionResponse() {
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

void NextionFlasher::getCommand() {
    if (is_port_open && is_connection_open) {
        //   writeCommand(GET_COMMAND);
        checkResponse();

        std::cout << "Frame size: " << std::dec << +buffer->data[1] << std::endl;
        std::cout << "bootloader: " << std::hex << +buffer->data[2] << std::endl;
    }
}

void NextionFlasher::getVersionCommand() {
    if (is_port_open && is_connection_open) {
        // writeCommand(GET_VERSION_COMMAND);
        checkResponse();
    }
}

void NextionFlasher::getIdCommand() {
    if (is_port_open && is_connection_open) {
        // writeCommand(GET_ID_COMMAND);
        checkResponse();
    }
}

void NextionFlasher::readMemoryCommand(uint32_t start_address, uint8_t length) {
    if (is_port_open && is_connection_open) {
        // writeCommand(READ_MEMORY_COMMAND);
        checkResponse();

        // writing_buffer->data[0] = *((uint8_t *) &start_address + 3);
        // writing_buffer->data[0] = *((uint8_t *) &start_address + 3);
        // writing_buffer->data[0] = *((uint8_t *) &start_address + 3);

        // uart->writeData(*((uint8_t *) &start_address + 2));
        // uart->writeData(*((uint8_t *) &start_address + 1));
        // uart->writeData(*((uint8_t *) &start_address + 0));

        // uart->writeData(0x08);
        writeAddress(start_address);
        checkResponse();

        //writeCommand(length);
        checkResponse();
    }
}

void NextionFlasher::goCommand(uint32_t address) {
    if (is_port_open && is_connection_open) {
        //writeCommand(GO_COMMAND);
        checkResponse();
        writeAddress(address);
        checkResponse();
    }
}

void NextionFlasher::flashCommand(uint32_t start_address, uint8_t *buffer, uint16_t length) {
    if (is_port_open && is_connection_open) {
        //writeCommand(WRITE_MEMORY_COMMAND);
        checkResponse();
        writeAddress(start_address);
        checkResponse();
        uart->addDataToBufferTX(length);
        uint8_t xor_checksum = 0x00;
        xor_checksum ^= length;
        for (int i = 0; i <= length; i++) {
            uart->addDataToBufferTX(buffer[i]);
            xor_checksum ^= buffer[i];
        }
        uart->addDataToBufferTX(xor_checksum);
        uart->writeData();
        checkResponse();
    }
}

void NextionFlasher::eraseCommand() {
    if (is_port_open && is_connection_open) {
        // writeCommand(ERASE_COMMAND);
        checkResponse();

        // writeCommand(FULL_CHIP_ERASE_COMMAND);
        checkResponse();
    }
}

void NextionFlasher::writeCommand(std::string command) {
    if (is_port_open) {
        //uart->addDataToBufferTX(command);
        //uart->addDataToBufferTX(~command);
        uart->addDataToBufferTX(command);
        uart->addDataToBufferTX(255);
        uart->addDataToBufferTX(255);
        uart->addDataToBufferTX(255);
        uart->writeData();
    }
}

void NextionFlasher::flashFile(FileReader::file_struct file) {
    //flashFile(file.data, file.size);
    std::string string = "";
    std::string cmd = "";

    int flashingBaudRate = 115200;

    std::string filesize_str = std::to_string(file.size);
    std::string baudrate_str = std::to_string(flashingBaudRate);
    cmd = "whmi-wri " + filesize_str + "," + baudrate_str + ",0";
    std::cout << cmd;
    writeCommand("");
    writeCommand(cmd);

    buffer->size = uart->waitForResponse(500);

    if ((buffer->size == 1) && (buffer->data[0] == 0x05)) {
        std::cout << "staring flashing from file";
    }

    uart->changeBaudRate(flashingBaudRate);

    std::cout << "file size " << std::dec << file.size << std::endl;

    uint32_t packets_amount = (file.size / 4096);
    for (int i = 0; i <= packets_amount; i++) {
        for (int j = 0; j < 4096; j++) {
            int byte_index = j + 4096 * i;
            if (byte_index == file.size) {
                goto out;
            }
            uart->addDataToBufferTX(file.data[byte_index]);
        }
        //  std::cout << std::dec << uart->writing_buffer.size << "\n";
        uart->writeData();
        buffer->size = uart->waitForFirstResponse(500);
        uart->writing_buffer.size = 0;

        if ((buffer->size == 1) && (buffer->data[0] == 0x05)) {
            std::cout << "Packet " << std::dec << i << "/" << packets_amount << " sent\n";
        } else {
            std::cout << "no confirming byte in response for flashing packet\n";
        }
    }
out:
    uart->writeData();
    buffer->size = uart->waitForFirstResponse(500);
    if ((buffer->size == 1) && (buffer->data[0] == 0x05)) {
        std::cout << "Packet " << std::dec << packets_amount << "/" << packets_amount << " sent\n";
    } else {
        std::cout << "no confirming byte in response for flashing packet\n";
    }
}

void NextionFlasher::flashFile(uint8_t *data, uint32_t size) {
    // if (is_port_open && is_connection_open) {
    //     uint16_t pages = size / 256;
    //     uint16_t unfull_page_size = size - pages * 256;
    //     uint32_t address = 0;
    //     eraseCommand();
    //     for (int page_counter = 0; page_counter < pages; page_counter++, address += 256) {
    //         flashCommand(0x8000000 + address, &data[address], 0xff);
    //     }
    //     flashCommand(0x8000000 + address, &data[address], unfull_page_size - 1);

    //     goCommand(0x8000000);
    // }
}

void NextionFlasher::writeAddress(uint32_t address) {
    uint8_t xor_checksum = 0x00;
    for (int i = 0; i < sizeof(uint32_t); i++) {
        uint8_t msb_byte = *((uint8_t *)&address + sizeof(uint32_t) - i - 1);
        uart->addDataToBufferTX(msb_byte);
        xor_checksum ^= msb_byte;
    }
    uart->addDataToBufferTX(xor_checksum);
    uart->writeData();
}

uint8_t NextionFlasher::checkResponse(ack_pos pos) {
    uint8_t ack_at_right_pos = false;

    if (is_port_open) {
        buffer->size = uart->waitForResponse(500);
        if (buffer->size > 0) {
            // switch (pos) {
            //     case NONE_ACK:
            //         ack_at_right_pos = true;
            //         break;
            //     case ACK_AT_BEGIN:
            //         if (buffer->data[0] == ACK) {
            //             ack_at_right_pos = true;
            //         } else {
            //             ack_at_right_pos = false;
            //         }
            //         break;
            //     case ACK_AT_END:
            //         if (buffer->data[buffer->size - 1] == ACK) {
            //             ack_at_right_pos = true;
            //         } else {
            //             ack_at_right_pos = false;
            //         }
            //         break;
            //     case ACK_AT_BEGIN_AND_END:
            //         if ((buffer->data[0] == ACK) && (buffer->data[buffer->size - 1] == ACK)) {
            //             ack_at_right_pos = true;
            //         } else {
            //             ack_at_right_pos = false;
            //         }
            //         break;
            // };
            std::cout << buffer->data;
        }
    }
    return ack_at_right_pos;
}

void NextionFlasher::connect() {
}

void NextionFlasher::readChip() {
}