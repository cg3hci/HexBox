#pragma once

#include <HMP/Meshing/types.hpp>
#include <HMP/Gui/SidebarWidget.hpp>
#include <cinolib/gl/key_bindings.hpp>

namespace HMP::Gui::Widgets
{

	class Commander final: public SidebarWidget
	{

	private:

		static constexpr cinolib::KeyBinding c_kbUndo{ GLFW_KEY_Z, GLFW_MOD_CONTROL };
		static constexpr cinolib::KeyBinding c_kbRedo{ GLFW_KEY_Z, GLFW_MOD_CONTROL | GLFW_MOD_SHIFT };

		void drawSidebar() override;

		bool keyPressed(const cinolib::KeyBinding& _key) override;

		void printUsage() const override;

	public:

		Commander();

	};

}