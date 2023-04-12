#pragma once

#include <HMP/Gui/Utils/Theme.hpp>
#include <cpputils/collections/Event.hpp>
#include <cpputils/mixins/ReferenceClass.hpp>

namespace HMP::Gui::Utils
{

    class Themer final: public cpputils::mixins::ReferenceClass
    {

    private:

        Theme m_theme;

        void applyImGui() const;

    public:

        mutable cpputils::collections::Event<Themer> onThemeChange{};

        void setTheme(const Theme& _theme);

        const Theme& operator*() const;
        const Theme* operator->() const;

    };

}