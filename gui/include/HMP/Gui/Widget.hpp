#pragma once

#include <cpputils/mixins/ReferenceClass.hpp>
#include <cinolib/gl/key_bindings.hpp>
#include <cinolib/gl/canvas_gui_item.h>
#include <cinolib/gl/glcanvas.h>
#include <cinolib/drawable_object.h>
#include <HMP/Utils/Serialization.hpp>
#include <HMP/Meshing/types.hpp>
#include <vector>

namespace HMP::Gui
{

	class App;

	class Widget: public cpputils::mixins::ReferenceClass, private cinolib::CanvasGuiItem
	{

	private:

		friend class App;

		App* m_app;

		void draw(const cinolib::GLcanvas& _canvas) final;

	protected:

		Widget();

		virtual ~Widget() = default;

		App& app();

		const App& app() const;

		virtual void printUsage() const;

		virtual void drawCanvas();

		virtual bool keyPressed(const cinolib::KeyBinding& _binding);

		virtual bool mouseClicked(bool _right);

		virtual bool mouseMoved(const Vec2& _position);

		virtual void cursorChanged();

		virtual void cameraChanged();

		virtual void actionApplied();

		virtual void actionPrepared();

		virtual void serialize(HMP::Utils::Serialization::Serializer& _serializer) const;

		virtual void deserialize(HMP::Utils::Serialization::Deserializer& _deserializer);

		virtual void attached();

		virtual std::vector<const cinolib::DrawableObject*> additionalDrawables() const;

		virtual std::vector<cinolib::GLcanvas::Font> additionalFonts(const std::vector<cinolib::GLcanvas::Font>& _fonts) const;

	};

}