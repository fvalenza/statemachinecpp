#ifndef BARE_PRINTER_HPP
#define BARE_PRINTER_HPP

#include <string>


class Printer {
public:
    void print(const std::string& string);
    void print(const std::string& string1, const std::string& string2);
};

#endif /* BARE_PRINTER_HPP */
