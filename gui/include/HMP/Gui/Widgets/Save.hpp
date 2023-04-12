#pragma once

#include <HMP/Gui/SidebarWidget.hpp>
#include <cinolib/gl/key_bindings.hpp>
#include <string>
#include <chrono>

namespace HMP::Gui::Widgets
{

    class Save final: public SidebarWidget
    {

    private:

        static constexpr cinolib::KeyBinding c_kbSave{ GLFW_KEY_S, GLFW_MOD_CONTROL };
        static constexpr cinolib::KeyBinding c_kbSaveNew{ GLFW_KEY_S, GLFW_MOD_CONTROL | GLFW_MOD_SHIFT };
        static constexpr cinolib::KeyBinding c_kbLoad{ GLFW_KEY_O, GLFW_MOD_CONTROL };

        std::string m_filename;
        std::chrono::time_point<std::chrono::steady_clock> m_lastTime;
        bool m_loaded;

        void apply(bool _load, const std::string& _filename);

        const std::string& filename() const;

        void drawSidebar() override;

        void printUsage() const override;

        bool keyPressed(const cinolib::KeyBinding& _key) override;

        void exportMesh(const std::string& _filename) const;

    public:

        explicit Save();

        void requestSave();

        void requestSaveNew();

        void requestLoad();

        void requestLoad(const std::string& _filename);

        void requestExportMesh();

    };

}