#ifndef WINDOWER_UTILITIES_SIGSCAN_HPP
#define WINDOWER_UTILITIES_SIGSCAN_HPP

#include <string>
#include <vector>

namespace windower::util
{

std::uint8_t* SigScan(const char* pattern, std::ptrdiff_t offset, const char* module_name);

}

#endif
