#pragma once

#include <HMP/Meshing/types.hpp>
#include <HMP/Gui/Widget.hpp>
#include <cinolib/gl/key_bindings.hpp>

namespace HMP::Gui::Widgets
{

    class DirectVertEdit final: public Widget
    {

    private:

        static constexpr cinolib::KeyBinding c_kbCancel{ GLFW_KEY_ESCAPE };
        static constexpr cinolib::KeyBinding c_kbTranslate{ GLFW_KEY_T };
        static constexpr cinolib::KeyBinding c_kbScale{ GLFW_KEY_S };
        static constexpr cinolib::KeyBinding c_kbRotate{ GLFW_KEY_R };
        static constexpr int c_kbOnX{ GLFW_KEY_LEFT_CONTROL };
        static constexpr int c_kbOnY{ GLFW_KEY_LEFT_SHIFT };
        static constexpr int c_kbOnZ{ GLFW_KEY_LEFT_ALT };

        enum class EKind
        {
            Translation, Rotation, Scale
        };

        bool m_pending;
        Vec2 m_centroid;
        EKind m_kind;
        bool m_onX, m_onY, m_onZ;
        Vec2 m_start;

        void printUsage() const override;

        bool mouseMoved(const Vec2& _position) override;

        bool keyPressed(const cinolib::KeyBinding& _key) override;

        void cameraChanged() override;

        bool mouseClicked(bool _right) override;

        void cancel();

        void apply();

        void drawCanvas() override;

        void update();

        void request(EKind _kind);

        void attached() override;

    public:

        float lineThickness{ 1.0f };
        float crossRadius{ 10.0f };
        float textSize{ 20.0f };
        Vec2 textMargin{ 10.0f };

        DirectVertEdit();

        bool pending() const;

    };

}