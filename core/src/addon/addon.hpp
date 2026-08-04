#ifndef WINDOWER_ADDON_ADDON_HPP
#define WINDOWER_ADDON_ADDON_HPP

#include "lua.hpp"
#include "package_manager.hpp"
#include "script_base.hpp"

#include <memory>
#include <string>

namespace windower
{

    class addon : public script_base
    {
    public:
        static std::shared_ptr<windower::package const> get_package(lua::state);

        addon(std::shared_ptr<windower::package const> const&);
        addon(addon const&) = delete; // Rule of 5: Prevent copies
        addon(addon&&) = delete; // Rule of 5: Prevent moves

        virtual ~addon();                        // <-- Changed to just 'virtual'

        addon& operator=(addon const&) = delete; // Rule of 5: Prevent copy assignment
        addon& operator=(addon&&) = delete; // Rule of 5: Prevent move assignment

        std::shared_ptr<windower::package const>
            find_dependency(lua::state, std::u8string_view) const override;

        std::shared_ptr<windower::package const> package() const;

    private:
        std::u8string m_package_name;
        mutable std::weak_ptr<windower::package const> m_package;
    };

}

#endif
