#include "util.h"
#include "types.h"
#include <iostream>

int main() {
    std::string s = stateStr(SystemState::IDLE);
    if (s != "IDLE") {
        std::cerr << "stateStr(SystemState::IDLE) returned '" << s << "'\n";
        return 1;
    }
    std::cout << "stateStr smoke test passed\n";
    return 0;
}
