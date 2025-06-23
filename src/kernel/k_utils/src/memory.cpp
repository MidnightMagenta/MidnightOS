#include "../../k_utils/include/memory.hpp"

void *utils::memset(void *ptr, int value, size_t num) { 
    for(char* mem = (char*)ptr; mem < (char*)(uintptr_t(ptr) + num); mem++){
        *mem = char(value);
    }
    return ptr;
}