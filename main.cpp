

#include "FileReader.h"
#include "NextionFlasher.h"
#include "vector"

int main(int argc, char *argv[]) {
    if (argc == 3) {
        NextionFlasher *flasher = new NextionFlasher(std::string(argv[2]));

        FileReader *file = new FileReader(argv[1]);

        flasher->flashFile(file->getFile());
    } else {
        std::cout << "Invalid parameters" << std::endl;
        std::cout << "Right use " << argv[0] << " <file> <port>" << std::endl;
        std::cout << "Example " << argv[0] << " expl.bin ttyUSB0" << std::endl;
    }
}

