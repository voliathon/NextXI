#ifndef WINDOWER_ADDON_SCRIPT_ENVIRONMENT_HPP
#define WINDOWER_ADDON_SCRIPT_ENVIRONMENT_HPP

#include "package_manager.hpp"
#include "script_base.hpp"

#include <string_view>

namespace windower
{

class script_environment : public script_base
{
public:
    script_environment() noexcept;

    virtual ~script_environment() = default;

    void run_until_idle();

    void execute(std::u8string_view) const;
    void evaluate(std::u8string_view) const;

    void reset();

    void initialize() const;

    std::shared_ptr<package const>
        find_dependency(lua::state, std::u8string_view) const override;
};

}

#endif