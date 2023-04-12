#include <HMP/Gui/Widgets/DirectVertEdit.hpp>

#include <HMP/Gui/Utils/Transform.hpp>
#include <HMP/Gui/Utils/Controls.hpp>
#include <HMP/Gui/Utils/Drawing.hpp>
#include <imgui.h>
#include <HMP/Gui/themer.hpp>
#include <HMP/Gui/Widgets/VertEdit.hpp>
#include <HMP/Gui/App.hpp>

namespace HMP::Gui::Widgets
{

	DirectVertEdit::DirectVertEdit():
		m_pending{ false }, m_centroid{}, m_kind{}, m_onX{}, m_onY{}, m_onZ{}, m_start{}
	{}

	void DirectVertEdit::attached()
	{
		app().vertEditWidget.onVidsChanged += [&]() {
			if (app().vertEditWidget.empty())
			{
				cancel();
			}
		};
	}

	void DirectVertEdit::printUsage() const
	{
		cinolib::print_binding(c_kbTranslate.name(), "translate selected vertices");
		cinolib::print_binding(c_kbScale.name(), "scale selected vertices");
		cinolib::print_binding(c_kbRotate.name(), "rotate selected vertices");
		cinolib::print_binding(cinolib::KeyBinding::key_name(c_kbOnX), "lock direct edit along X (hold down)");
		cinolib::print_binding(cinolib::KeyBinding::key_name(c_kbOnY), "lock direct edit along Y (hold down)");
		cinolib::print_binding(cinolib::KeyBinding::key_name(c_kbOnZ), "lock direct edit along Z (hold down)");
		cinolib::print_binding(c_kbCancel.name(), "cancel direct edit");
	}

	bool DirectVertEdit::mouseMoved(const Vec2& _position)
	{
		if (m_pending)
		{
			update();
			return true;
		}
		return false;
	}

	bool DirectVertEdit::keyPressed(const cinolib::KeyBinding& _key)
	{
		if (_key == c_kbTranslate)
		{
			request(Widgets::DirectVertEdit::EKind::Translation);
		}
		else if (_key == c_kbScale)
		{
			request(Widgets::DirectVertEdit::EKind::Scale);
		}
		else if (_key == c_kbRotate)
		{
			request(Widgets::DirectVertEdit::EKind::Rotation);
		}
		else if (_key == c_kbCancel)
		{
			cancel();
		}
		else
		{
			return false;
		}
		return true;
	}

	void DirectVertEdit::cameraChanged()
	{
		update();
	}

	bool DirectVertEdit::mouseClicked(bool _right)
	{
		if (m_pending)
		{
			if (_right)
			{
				cancel();
			}
			else
			{
				apply();
			}
			return true;
		}
		return false;
	}

	void DirectVertEdit::request(EKind _kind)
	{
		if (app().vertEditWidget.empty() && !m_pending)
		{
			return;
		}
		const bool wasPending{ m_pending };
		m_pending = false;
		app().vertEditWidget.applyAction();
		m_pending = !wasPending || m_kind != _kind;
		m_onX = m_onY = m_onZ = false;
		GLdouble depth;
		app().canvas.project(app().vertEditWidget.centroid(), m_centroid, depth);
		m_start = app().mouse;
		m_kind = _kind;
		Utils::Transform& transform{ app().vertEditWidget.transform() };
		transform.translation = { 0.0 };
		transform.scale = { 1.0 };
		transform.rotation = { 0.0 };
		app().vertEditWidget.applyTransform();
		if (wasPending != m_pending)
		{
			app().updateCursor();
		}
	}

	void DirectVertEdit::update()
	{
		if (!m_pending)
		{
			return;
		}
		const bool
			_onX{ glfwGetKey(app().canvas.window, c_kbOnX) == GLFW_PRESS },
			_onY{ glfwGetKey(app().canvas.window, c_kbOnY) == GLFW_PRESS },
			_onZ{ glfwGetKey(app().canvas.window, c_kbOnZ) == GLFW_PRESS };
		const Vec up{ app().canvas.camera.view.normUp() };
		const Vec right{ app().canvas.camera.view.normRight() };
		const Vec forward{ app().canvas.camera.view.normBack() };
		Utils::Transform& transform{ app().vertEditWidget.transform() };
		transform.translation = { 0.0 };
		transform.scale = { 1.0 };
		transform.rotation = { 0.0 };
		m_onX = _onX;
		m_onY = _onY;
		m_onZ = _onZ;
		switch (m_kind)
		{
			case EKind::Translation:
			{
				if (m_onZ || (m_onX && m_onY))
				{
					break;
				}
				Vec2 diff{ app().mouse - m_start };
				if (m_onY)
				{
					diff.x() = 0.0;
				}
				if (m_onX)
				{
					diff.y() = 0.0;
				}
				const Vec2 newCentroid{ m_centroid + diff };
				const cinolib::Ray ray{ app().canvas.eye_to_screen_ray(newCentroid) };
				Real denom = forward.dot(ray.dir());
				if (!Utils::Transform::isNull(denom))
				{
					const Real t = (transform.origin - ray.begin()).dot(forward) / denom;
					const Vec newCentroid3d{ ray.begin() + ray.dir() * t };
					transform.translation = newCentroid3d - transform.origin;
				}
			}
			break;
			case EKind::Scale:
			{
				const Vec2 diff{ app().mouse - m_centroid };
				const Vec2 startDiff{ m_start - m_centroid };
				if (!m_onX && !m_onY && !m_onZ)
				{
					const auto scaleDim{ [&diff, &startDiff](unsigned int _dim) {
						const Real dd{diff[_dim]}, dsd{startDiff[_dim]};
						return Utils::Transform::isNull(dsd) ? 1.0 : std::abs(dd / dsd);
					} };
					transform.scale = right * scaleDim(0) + up * scaleDim(1) + forward;
					transform.scale.x() = std::abs(transform.scale.x());
					transform.scale.y() = std::abs(transform.scale.y());
					transform.scale.z() = std::abs(transform.scale.z());
				}
				else
				{
					const Real dn{ diff.norm() }, dsn{ startDiff.norm() };
					const Real scale{ Utils::Transform::isNull(dsn) ? 1.0 : (dn / dsn) };
					if (m_onX)
					{
						transform.scale.x() = scale;
					}
					if (m_onY)
					{
						transform.scale.y() = scale;
					}
					if (m_onZ)
					{
						transform.scale.z() = scale;
					}
				}
			}
			break;
			case EKind::Rotation:
			{
				Vec axis{};
				if (!m_onX && !m_onY && !m_onZ)
				{
					axis = forward;
				}
				else if (m_onX && !m_onY && !m_onZ)
				{
					axis = { 1,0,0 };
				}
				else if (!m_onX && m_onY && !m_onZ)
				{
					axis = { 0,1,0 };
				}
				else if (!m_onX && !m_onY && m_onZ)
				{
					axis = { 0,0,1 };
				}
				else
				{
					break;
				}
				if (axis.dot(forward) < 0)
				{
					axis = -axis;
				}
				const Vec2 startDir{ Utils::Transform::dir(m_centroid, m_start) };
				const Vec2 dir{ Utils::Transform::dir(m_centroid, app().mouse) };
				const Real angle{ Utils::Transform::angle(startDir, dir) };
				const Mat3 mat{ Utils::Transform::rotationMat(axis, angle) };
				transform.rotation = Utils::Transform::rotationMatToVec(mat);
			}
			break;
		}
		app().vertEditWidget.applyTransform();
	}

	void DirectVertEdit::apply()
	{
		if (!m_pending)
		{
			return;
		}
		m_pending = false;
		app().vertEditWidget.applyAction();
		app().updateCursor();
	}

	void DirectVertEdit::cancel()
	{
		if (!m_pending)
		{
			return;
		}
		m_pending = false;
		app().vertEditWidget.cancel();
		app().updateCursor();
	}

	bool DirectVertEdit::pending() const
	{
		return m_pending;
	}

	void DirectVertEdit::drawCanvas()
	{
		if (!m_pending)
		{
			return;
		}
		const Vec2 mouse{ app().mouse };
		const float
			textSize{ this->textSize * themer->ovScale },
			lineThickness{ this->lineThickness * themer->ovScale },
			crossRadius{ this->crossRadius * themer->ovScale };
		const Vec2 textMargin{ this->textMargin * static_cast<Real>(themer->ovScale) };
		using Utils::Controls::toImVec2;
		using Utils::Controls::toImVec4;
		using Utils::Drawing::toImVec4;
		using namespace Utils::Drawing;
		ImDrawList& drawList{ *ImGui::GetWindowDrawList() };
		const Real maxLen{ Vec2{static_cast<Real>(app().canvas.canvas_width()), static_cast<Real>(app().canvas.height())}.norm() };
		const char* verticesLit{ app().vertEditWidget.vids().size() == 1 ? "vertex" : "vertices" };
		const int vertexCount{ static_cast<int>(app().vertEditWidget.vids().size()) };
		switch (m_kind)
		{
			case EKind::Rotation:
			{
				const char* axisStr;
				if (!m_onX && !m_onY && !m_onZ)
				{
					axisStr = "";
				}
				else if (m_onX && !m_onY && !m_onZ)
				{
					axisStr = "X";
				}
				else if (!m_onX && m_onY && !m_onZ)
				{
					axisStr = "Y";
				}
				else if (!m_onX && !m_onY && m_onZ)
				{
					axisStr = "Z";
				}
				else
				{
					text(drawList, "???", toImVec2(m_centroid + textMargin), textSize, themer->ovErr, EAlign::LeftTop, EAlign::LeftTop);
					break;
				}
				text(drawList, axisStr, toImVec2(m_centroid + textMargin), textSize, themer->ovMut, EAlign::LeftTop, EAlign::LeftTop);
				const Vec2 startDir{ Utils::Transform::dir(m_centroid, m_start) };
				const Vec2 dir{ Utils::Transform::dir(m_centroid, mouse) };
				line(drawList, { toImVec2(m_centroid), toImVec2(m_centroid + startDir * maxLen) }, themer->ovMut, lineThickness);
				line(drawList, { toImVec2(m_centroid), toImVec2(m_centroid + dir * maxLen) }, themer->ovHi, lineThickness);
				const Vec rot{ app().vertEditWidget.transform().rotation };
				ImGui::TextColored(toImVec4(themer->ovWarn), "Rotating %d %s by %1.f,%1.f,%1.f degrees via direct manipulation", vertexCount, verticesLit, rot.x(), rot.y(), rot.z());
			}
			break;
			case EKind::Scale:
			{
				if (m_onX || m_onY || m_onZ)
				{
					std::string axisStr{};
					if (m_onX) { axisStr += "X"; }
					if (m_onY) { axisStr += "Y"; }
					if (m_onZ) { axisStr += "Z"; }
					text(drawList, axisStr.c_str(), toImVec2(m_centroid + textMargin), textSize, themer->ovMut, EAlign::LeftTop, EAlign::LeftTop);
				}
				const Real startRadius{ m_centroid.dist(m_start) };
				const Real radius{ m_centroid.dist(mouse) };
				circle(drawList, toImVec2(m_centroid), static_cast<float>(startRadius), themer->ovMut, lineThickness);
				circle(drawList, toImVec2(m_centroid), static_cast<float>(radius), themer->ovHi, lineThickness);
				const Vec scl{ app().vertEditWidget.transform().scale * 100.0 };
				ImGui::TextColored(toImVec4(themer->ovWarn), "Scaling %d %s by %2.f,%2.f,%2.f%% via direct manipulation", vertexCount, verticesLit, scl.x(), scl.y(), scl.z());
			}
			break;
			case EKind::Translation:
			{
				if (m_onX && !m_onY && !m_onZ)
				{
					line(drawList, { toImVec2(m_start - Vec2{ maxLen, 0 }), toImVec2(m_start + Vec2{ maxLen, 0 }) }, themer->ovMut, lineThickness);
					line(drawList, { toImVec2(m_start - Vec2{ 0, crossRadius }), toImVec2(m_start + Vec2{ 0, crossRadius }) }, themer->ovMut, lineThickness);
					line(drawList, { toImVec2(Vec2{ mouse.x(), m_start.y() } - Vec2{ 0, crossRadius }), toImVec2(Vec2{ mouse.x(), m_start.y() } + Vec2{ 0, crossRadius }) }, themer->ovHi, lineThickness);
					text(drawList, "X", toImVec2(m_start + textMargin), textSize, themer->ovMut, EAlign::LeftTop, EAlign::LeftTop);
				}
				else if (!m_onX && m_onY && !m_onZ)
				{
					line(drawList, { toImVec2(m_start - Vec2{ 0, maxLen }), toImVec2(m_start + Vec2{ 0, maxLen }) }, themer->ovMut, lineThickness);
					line(drawList, { toImVec2(m_start - Vec2{ crossRadius, 0 }), toImVec2(m_start + Vec2{ crossRadius, 0 }) }, themer->ovMut, lineThickness);
					line(drawList, { toImVec2(Vec2{ m_start.x(), mouse.y() } - Vec2{ crossRadius, 0 }), toImVec2(Vec2{ m_start.x(), mouse.y() } + Vec2{ crossRadius, 0 }) }, themer->ovHi, lineThickness);
					text(drawList, "Y", toImVec2(m_start + textMargin), textSize, themer->ovMut, EAlign::LeftTop, EAlign::LeftTop);
				}
				else if (!m_onX && !m_onY && !m_onZ)
				{
					cross(drawList, toImVec2(m_start), crossRadius, themer->ovMut, lineThickness);
					line(drawList, { toImVec2(m_start), toImVec2(mouse) }, themer->ovHi, lineThickness);
				}
				else
				{
					line(drawList, { toImVec2(m_start - Vec2{ crossRadius, 0 }), toImVec2(m_start + Vec2{ crossRadius, 0 }) }, themer->ovMut, lineThickness);
					line(drawList, { toImVec2(m_start - Vec2{ 0, crossRadius }), toImVec2(m_start + Vec2{ 0, crossRadius }) }, themer->ovMut, lineThickness);
					text(drawList, "???", toImVec2(m_start + textMargin), textSize, themer->ovErr, EAlign::LeftTop, EAlign::LeftTop);
				}
				const Vec trs{ app().vertEditWidget.transform().translation };
				ImGui::TextColored(toImVec4(themer->ovWarn), "Translating %d %s by %.3f,%.3f,%.3f via direct manipulation", vertexCount, verticesLit, trs.x(), trs.y(), trs.z());
			}
			break;
		}
	}

}