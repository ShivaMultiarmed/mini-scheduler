#include <algorithm>
#include <cstdint>
#include <format>
#include <iostream>
#include "TaskGroup.h"

void emulateWork(const std::string &workTitle, const uint32_t complexity = 1e9) {
    std::cout << std::format("Work {} is being done\n", workTitle);
    const uint32_t stepSize = 1000;
    for (uint32_t i = 0; i < complexity; ++i) {
        if (i % stepSize == 0) {
            const uint32_t step = i / stepSize;
            std::cout << std::format("Work {} step {}\n", workTitle, step);
        }
    }
    std::cout << std::format("Work {} has been finished\n", workTitle);
}

void fib(
    std::shared_ptr<Pool> pool,
    uint32_t n,
    uint32_t *r
    ) {
    if (n == 0) {
        *r = 0;
        return;
    }
    if (n == 1 || n == 2) {
        *r = 1;
        return;
    }
    auto parentGroup = TaskGroup(pool);
    uint32_t a, b;
    parentGroup.submit([&pool, n, &a](){
        fib(pool, n - 1, &a);
    });
    parentGroup.submit([&pool, n, &b](){
        fib(pool, n - 2, &b);
    });
    parentGroup.wait();
    *r = a + b;
}

void nestedCompression() {
    auto pool = std::make_shared<Pool>(2);
    pool->run();
    auto parentGroup = TaskGroup(pool);
    parentGroup.submit([pool]() {
        std::cout << "An archive is being compressed.\n";
        auto childGroup = TaskGroup(pool);
        const uint32_t fileCount = 4;
        for (uint32_t i = 0; i < fileCount; ++i) {
            childGroup.submit([i]() {
                std::cout << std::format("A file {} is being compressed.\n", i);
            });
        }
        childGroup.wait();
    });
    parentGroup.wait();
}

void fibExp() {
    auto pool = std::make_shared<Pool>(2);
    pool->run();
    uint32_t n = 10;
    uint32_t t;
    auto tGroup = TaskGroup(pool);
    tGroup.submit([&pool, &t, n]() {
        fib(pool, n, &t);
    });
    tGroup.wait();
    std::cout << std::format("Fib #{} is {}.\n", n, t);
}

int main() {
    nestedCompression();
    return 0;
}