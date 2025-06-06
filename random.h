#pragma once

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <iostream>

class Random {
private:
    FILE * fp;

public:
    Random() {
        fp = fopen("/dev/urandom", "rb");
        if (!fp) {
            std::cerr << "Failed to open /dev/urandom" << std::endl;
            exit(EXIT_FAILURE);
        }
    }

    ~Random() {
        fclose(fp);
    }

    uint64_t rand64() {
        uint64_t value;
        if(fread(&value, sizeof(uint64_t), 1, fp) != 1) {
            std::cerr << "Failed to read data from /dev/urandom" << std::endl;
            exit(EXIT_FAILURE);
        }
        return value;
    }
};
