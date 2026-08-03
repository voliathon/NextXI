#ifndef WINDOWER_UI_TEXTURE_HPP
#define WINDOWER_UI_TEXTURE_HPP

#include "ui/patch.hpp"
#include "ui/texture_token.hpp"

namespace windower::ui
{

class texture
{
public:
    texture_token token;
    patch patch;
};

}

#endif