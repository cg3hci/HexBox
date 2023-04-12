#include <HMP/Gui/Widgets/Save.hpp>

#include <imgui.h>
#include <HMP/Gui/Utils/Controls.hpp>
#include <HMP/Gui/Utils/FilePicking.hpp>
#include <HMP/Gui/themer.hpp>
#include <HMP/Gui/App.hpp>
#include <chrono>

namespace HMP::Gui::Widgets
{

    Save::Save(): SidebarWidget{ "Save" }, m_filename{}, m_loaded{ false }
    {}

    void Save::printUsage() const
    {
        cinolib::print_binding(c_kbSave.name(), "save");
        cinolib::print_binding(c_kbSaveNew.name(), "save new");
        cinolib::print_binding(c_kbLoad.name(), "load");
    }

    bool Save::keyPressed(const cinolib::KeyBinding& _key)
    {
        if (_key == c_kbSave)
        {
            requestSave();
        }
        else if (_key == c_kbSaveNew)
        {
            requestSaveNew();
        }
        else if (_key == c_kbLoad)
        {
            requestLoad();
        }
        else
        {
            return false;
        }
        return true;
    }

    void Save::apply(bool _load, const std::string& _filename)
    {
        m_loaded = _load;
        m_filename = _filename;
        m_lastTime = std::chrono::steady_clock::now();
        if (_load)
        {
            app().deserialize(_filename);
        }
        else
        {
            app().serialize(_filename);
        }
    }

    const std::string& Save::filename() const
    {
        return m_filename;
    }

    void Save::requestSave()
    {
        if (m_filename.empty())
        {
            requestSaveNew();
        }
        else
        {
            apply(false, m_filename);
        }
    }

    void Save::requestSaveNew()
    {
        const std::optional<std::string> filename{ Utils::FilePicking::save("hmp") };
        if (filename)
        {
            apply(false, *filename);
        }
    }

    void Save::requestLoad()
    {
        const std::optional<std::string> filename{ Utils::FilePicking::open() };
        if (filename)
        {
            apply(true, *filename);
        }
    }

    void Save::requestLoad(const std::string& _filename)
    {
        apply(true, _filename);
    }

    void Save::requestExportMesh()
    {
        const std::optional<std::string> filename{ Utils::FilePicking::save("mesh") };
        if (filename)
        {
            exportMesh(*filename);
        }
    }

    void Save::exportMesh(const std::string& _filename) const
    {
        Meshing::Mesher::Mesh mesh{ app().mesh };
        for (Id pidPlusOne{ mesh.num_polys() }; pidPlusOne > 0; --pidPlusOne)
        {
            if (!app().mesher.shown(pidPlusOne - 1))
            {
                mesh.poly_remove(pidPlusOne - 1, true);
            }
        }
        mesh.save(_filename.c_str());
    }

    void Save::drawSidebar()
    {
        if (!m_filename.empty())
        {
            const int elapsedMins{ static_cast<int>(std::chrono::duration_cast<std::chrono::minutes>(std::chrono::steady_clock::now() - m_lastTime).count()) };
            const char* actionStr{ m_loaded ? "Loaded" : "Saved" };
            if (elapsedMins < 1)
            {
                ImGui::TextColored(themer->sbOk, "%s less than a minute ago", actionStr);
            }
            else if (elapsedMins == 1)
            {
                ImGui::TextColored(themer->sbOk, "%s a minute ago", actionStr);
            }
            else
            {
                ImGui::TextColored(elapsedMins > 5 ? themer->sbWarn : themer->sbOk, "%s %d minutes ago", actionStr, elapsedMins);
            }
            ImGui::Text("%s", m_filename.c_str());
            ImGui::Spacing();
            if (ImGui::Button("Save"))
            {
                requestSave();
            }
            ImGui::SameLine();
            if (ImGui::Button("Save new"))
            {
                requestSaveNew();
            }
        }
        else
        {
            ImGui::TextColored(themer->sbWarn, "Not saved");
            ImGui::Spacing();
            if (ImGui::Button("Save"))
            {
                requestSaveNew();
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Load"))
        {
            requestLoad();
        }
        ImGui::SameLine();
        if (ImGui::Button("Export mesh"))
        {
            requestExportMesh();
        }
    }

}