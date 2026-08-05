#ifndef WINDOWER_UTILITES_XML_HPP
#define WINDOWER_UTILITES_XML_HPP

#include <pugixml.hpp>

#include <filesystem>
#include <iosfwd>

namespace windower
{

void check(
    pugi::xml_parse_result const& result, std::ifstream& stream,
    std::filesystem::path const& path = {});

}

#endif