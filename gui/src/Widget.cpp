#include <HMP/Gui/Widget.hpp>

#include <HMP/Gui/App.hpp>

namespace HMP::Gui
{

	Widget::Widget(): m_app{}
	{}

	App& Widget::app()
	{
		return *m_app;
	}

	const App& Widget::app() const
	{
		return *m_app;
	}

	void Widget::printUsage() const
	{}

	void Widget::draw(const cinolib::GLcanvas& _canvas)
	{
		drawCanvas();
	}

	void Widget::drawCanvas()
	{}

	bool Widget::keyPressed(const cinolib::KeyBinding& _binding)
	{
		return false;
	}

	bool Widget::mouseClicked(bool _right)
	{
		return false;
	}

	bool Widget::mouseMoved(const Vec2& _position)
	{
		return false;
	}

	void Widget::cursorChanged()
	{}

	void Widget::cameraChanged()
	{}

	void Widget::actionApplied()
	{}

	void Widget::actionPrepared()
	{}

	void Widget::serialize(HMP::Utils::Serialization::Serializer& _serializer) const
	{}

	void Widget::deserialize(HMP::Utils::Serialization::Deserializer& _deserializer)
	{}

	void Widget::attached()
	{}

	std::vector<const cinolib::DrawableObject*> Widget::additionalDrawables() const
	{
		return {};
	}

	std::vector<cinolib::GLcanvas::Font> Widget::additionalFonts(const std::vector<cinolib::GLcanvas::Font>& _fonts) const
	{
		return {};
	}

}