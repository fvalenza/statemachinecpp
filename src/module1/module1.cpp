#include "module1.hpp"
#include <iostream>

std::string concatString(const std::string string1, const std::string string2)
{
    return string1 + string2;
}

void Printer::print(const std::string& string)
{
    std::cout << string << std::endl;
}

void Printer::print(const std::string& string1, const std::string& string2)
{
    std::cout << concatString(string1, string2) << std::endl;
}
