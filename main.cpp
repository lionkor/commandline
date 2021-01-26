#include "commandline.h"

int main() {
    Commandline com;
    while (true) {
        if (com.has_command()) {
            auto command = com.get_command();
            com.write("got command: " + command);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        com.write("this is a message written with com.write");
    }
}
