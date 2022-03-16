#include "commandline.h"

int main(int argc, char** argv) {
    Commandline com;
    // com.enable_key_debug();
    if (argc > 1) {
        com.set_prompt(argv[1]);
    }
    com.enable_history();
    com.set_history_limit(5);

    com.on_autocomplete = [](Commandline& com, std::string stub, int pos) -> std::vector<std::string> {
        if (stub == "hello") {
            return { "hello world" };
        } else if (stub.empty()) {
            return { "a", "b", "c", "d" };
            //return { "hello world", "exit" };
        } else {
            return {};
        }
    };

    while (true) {
        if (com.has_command()) {
            auto command = com.get_command();
            com.write(command);
            if (command == "exit") {
                break;
            }
        }
        // this sleep is necessary in order to simulate a system load.
        // usually, instead of writing a message here, a message would
        // be written as the result of some internal program event.
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        com.write("this is a message written with com.write");
    }
}
