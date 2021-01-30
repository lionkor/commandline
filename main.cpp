#include "commandline.h"

int main(int argc, char** argv) {
    Commandline com;
    if (argc > 1) {
        com.set_prompt(argv[1]);
    }
    com.enable_history();
    com.set_history_limit(5);
    while (true) {
        if (com.has_command()) {
            auto command = com.get_command();
            com.write(command);
            if (command == "exit") {
                break;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        com.write("this is a message written with com.write");
    }
}
