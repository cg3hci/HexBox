#pragma once

#include <imgui.h>
#include <cinolib/gl/glcanvas.h>
#include <HMP/Meshing/types.hpp>
#include <array>
#include <algorithm>
#include <cpputils/range/of.hpp>
#include <optional>

namespace HMP::Gui::Utils::Drawing
{

	enum class EAlign
	{
		LeftTop, Middle, RightBottom
	};

	std::optional<ImVec2> project(const cinolib::GLcanvas& _canvas, const Vec& _point);

	template<I TSize>
	std::optional<std::array<ImVec2, TSize>> project(const cinolib::GLcanvas& _canvas, const std::array<Vec, TSize>& _points)
	{
		const std::array<std::optional<ImVec2>, TSize> points{ cpputils::range::of(_points).map([&](const Vec& _point) {
			return project(_canvas, _point);
		}).toArray() };
		if (cpputils::range::of(points).filter([](const auto& _p) {return !_p.has_value(); }).empty())
		{
			return cpputils::range::of(points).dereference().toArray();
		}
		return std::nullopt;
	}

	constexpr ImU32 toByte(float _colorComponent)
	{
		return std::clamp<ImU32>(static_cast<ImU32>(_colorComponent * 255.0f + 0.5f), 0, 255);
	}

	constexpr float fromByte(ImU32 _colorComponent)
	{
		return static_cast<float>(_colorComponent) / 255.0f;
	}

	constexpr ImU32 toImU32(const cinolib::Color& _color)
	{
		return IM_COL32(toByte(_color.r()), toByte(_color.g()), toByte(_color.b()), toByte(_color.a()));
	}

	constexpr cinolib::Color toColor(const ImU32 _color)
	{
		return cinolib::Color{
			fromByte((_color >> IM_COL32_R_SHIFT) & 0xFF),
			fromByte((_color >> IM_COL32_G_SHIFT) & 0xFF),
			fromByte((_color >> IM_COL32_B_SHIFT) & 0xFF),
			fromByte((_color >> IM_COL32_A_SHIFT) & 0xFF)
		};
	}

	constexpr ImVec4 toImVec4(const ImU32 _color)
	{
		return ImVec4{
			fromByte((_color >> IM_COL32_R_SHIFT) & 0xFF),
			fromByte((_color >> IM_COL32_G_SHIFT) & 0xFF),
			fromByte((_color >> IM_COL32_B_SHIFT) & 0xFF),
			fromByte((_color >> IM_COL32_A_SHIFT) & 0xFF)
		};
	}

	void dashedLine(ImDrawList& _drawList, const EdgeVertData<ImVec2>& _verts, ImU32 _color, float _thickness = 1.0f, float _spacing = 10.0f);

	void line(ImDrawList& _drawList, const EdgeVertData<ImVec2>& _verts, ImU32 _color, float _thickness = 1.0f);

	void circle(ImDrawList& _drawList, const ImVec2& _center, float _radius, ImU32 _color, float _thickness = 1.0f);

	void circle(ImDrawList& _drawList, const ImVec2& _center, float _radius, ImU32 _color, float _thickness, int _segments);

	void circleFilled(ImDrawList& _drawList, const ImVec2& _center, float _radius, ImU32 _color);
	
	void circleFilled(ImDrawList& _drawList, const ImVec2& _center, float _radius, ImU32 _color, int _segments);

	void cross(ImDrawList& _drawList, const ImVec2& _center, float _radius, ImU32 _color, float _thickness = 1.0f);

	void text(ImDrawList& _drawList, const char* _text, const ImVec2& _position, float _size, ImU32 _color, EAlign _hAlign = EAlign::Middle, EAlign _vAlign = EAlign::Middle);

	void quad(ImDrawList& _drawList, const QuadVertData<ImVec2>& _verts, ImU32 _color, float _thickness = 1.0f);

	void quadFilled(ImDrawList& _drawList, const QuadVertData<ImVec2>& _verts, ImU32 _color);

	void rect(ImDrawList& _drawList, const ImVec2& _min, const ImVec2& _max, ImU32 _color, float _thickness = 1.0f);

	void dashedRect(ImDrawList& _drawList, const ImVec2& _min, const ImVec2& _max, ImU32 _color, float _thickness = 1.0f, float _spacing = 10.0f);

	void rectFilled(ImDrawList& _drawList, const ImVec2& _min, const ImVec2& _max, ImU32 _color);

	void dashedLine(ImDrawList& _drawList, const std::optional<EdgeVertData<ImVec2>>& _verts, ImU32 _color, float _thickness = 1.0f, float _spacing = 10.0f);

	void line(ImDrawList& _drawList, const std::optional<EdgeVertData<ImVec2>>& _verts, ImU32 _color, float _thickness = 1.0f);

	void circle(ImDrawList& _drawList, const std::optional<ImVec2>& _center, float _radius, ImU32 _color, float _thickness = 1.0f);

	void circleFilled(ImDrawList& _drawList, const std::optional<ImVec2>& _center, float _radius, ImU32 _color);

	void circle(ImDrawList& _drawList, const std::optional<ImVec2>& _center, float _radius, ImU32 _color, float _thickness, int _segments);

	void circleFilled(ImDrawList& _drawList, const std::optional<ImVec2>& _center, float _radius, ImU32 _color, int _segments);

	void cross(ImDrawList& _drawList, const std::optional<ImVec2>& _center, float _radius, ImU32 _color, float _thickness = 1.0f);

	void text(ImDrawList& _drawList, const char* _text, const std::optional < ImVec2>& _position, float _size, ImU32 _color, EAlign _hAlign = EAlign::Middle, EAlign _vAlign = EAlign::Middle);

	void quad(ImDrawList& _drawList, const std::optional<QuadVertData<ImVec2>>& _verts, ImU32 _color, float _thickness = 1.0f);

	void quadFilled(ImDrawList& _drawList, const std::optional<QuadVertData<ImVec2>>& _verts, ImU32 _color);

	void rect(ImDrawList& _drawList, const std::optional<ImVec2>& _min, const std::optional<ImVec2>& _max, ImU32 _color, float _thickness = 1.0f);

	void dashedRect(ImDrawList& _drawList, const std::optional<ImVec2>& _min, const std::optional<ImVec2>& _max, ImU32 _color, float _thickness = 1.0f, float _spacing = 10.0f);

	void rectFilled(ImDrawList& _drawList, const std::optional<ImVec2>& _min, const std::optional<ImVec2>& _max, ImU32 _color);

	bool cull(const ImDrawList& _drawList, const ImVec2& _min, const ImVec2& _max);

	bool cull(const ImDrawList& _drawList, const ImVec2& _center, float _halfSize);

	bool cull(const ImDrawList& _drawList, const QuadVertData<ImVec2>& _verts);

	bool cull(const ImDrawList& _drawList, const EdgeVertData<ImVec2>& _verts);

}