#include <iostream>
#include "range_walk.hpp"
#include <thread>

bool adrgfs = []() {
    for (int i : RangeInt(12, -6))
        std::cout << i << std::endl;
    return false;
}();
int main()
{
    
    

}
