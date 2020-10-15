

#include "FileReader.h"
#include "NextionFlasher.h"
#include "vector"

int main(int argc, char *argv[]) {
    if (argc == 2) {
        NextionFlasher *flasher = new NextionFlasher(std::string(argv[1]));
        flasher->connect();
    } else if (argc == 3) {
        NextionFlasher *flasher = new NextionFlasher(std::string(argv[2]));
        flasher->connect();

        FileReader *file = new FileReader(argv[1]);

        flasher->flashFile(file->getFile());
    } else if (argc == 4) {
        NextionFlasher *flasher = new NextionFlasher(std::string(argv[2]));
        flasher->connect();
        flasher->setFlashingBaudRate(std::stoi(argv[3]));

        FileReader *file = new FileReader(argv[1]);

        flasher->flashFile(file->getFile());
    } else {
        std::cout << "Invalid parameters\n" << std::endl;
        std::cout << "Right use for device info:" << argv[0] << "<port>" << std::endl;
        std::cout << "Example: " << argv[0] << " COM4\n" << std::endl;
        std::cout << "Right use for flash:\n" << argv[0] << " <file> <port> <baud_rate>" << std::endl;
        std::cout << "  <baud_rate> is optional, for default program use fastes (921600)" << std::endl;
        std::cout << "Examples: " << std::endl;
        std::cout << "  " << argv[0] << " expl.bin ttyUSB0" << std::endl;
        std::cout << "  " << argv[0] << " expl.bin ttyUSB0 9600" << std::endl;
    }
}
