#include <cstdint>
#include <format>
#include <iostream>
#include "Pool.h"

void emulateWork(const std::string& workTitle, const uint32_t complexity = 1e9) {
    std::cout << std::format("Work {} is being done\n", workTitle);
    const uint32_t stepSize = 1000000;
    for (uint32_t i = 0; i < complexity; ++i) {
        if (i % stepSize == 0) {
            const uint32_t step = i / 1000000;
            std::cout << std::format("Work {} step {}\n", workTitle, step);
        }
    }
    std::cout << std::format("Work {} has been finished\n", workTitle);
}

int main() {
    Pool p;
    p.run();
    p.submit([](){
       emulateWork("Compressing image", 1e9);
    });
    p.submit([](){
       emulateWork("Sending an API call", 1e7);
    });
    return 0;
}