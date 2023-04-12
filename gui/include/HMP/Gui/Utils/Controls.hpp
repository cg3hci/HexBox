#pragma once 

#include <HMP/Meshing/types.hpp>
#include <imgui.h>
#include <cinolib/color.h>
#include <limits>
#include <type_traits>
#include <initializer_list>

namespace HMP::Gui::Utils::Controls
{

	bool dragVec(const char* _label, Vec& _vec, Real _speed, Real _min = -std::numeric_limits<Real>::infinity(), Real _max = std::numeric_limits<Real>::infinity(), const char* _format = "%.3f");

	bool dragTranslationVec(const char* _label, Vec& _translation, Real _sceneSize);

	bool dragScaleVec(const char* _label, Vec& _scale, Real _defScale = 1.0);

	bool dragScale(const char* _label, Real& _scale, Real _defScale = 1.0);

	bool dragRotation(const char* _label, Vec& _rotation);

	bool colorButton(const char* _label, cinolib::Color& _color);

	bool sliderI(const char* _label, I& _value, I _min = 0, I _max = 10);

	bool sliderReal(const char* _label, Real& _value, Real _min = 0.0, Real _max = 1.0, bool _logarithmic = false, const char* _format = "%.3f");

	bool sliderPercentage(const char* _label, Real& _value, Real _min = 0.0, Real _max = 1.0, const char* _format = "%.2f%%");

	bool sliderPercentage(const char* _label, float& _value, float _min = 0.0, float _max = 1.0, const char* _format = "%.2f%%");

	template<typename TEnum> requires std::is_enum_v<TEnum>
	bool combo(const char* _label, TEnum& _value, const std::initializer_list<const char*>& _values);

	bool disabledButton(const char* _label, bool _enabled = false);

	bool disabledSmallButton(const char* _label, bool _enabled = false);

	constexpr ImVec4 toImVec4(const cinolib::Color& _color)
	{
		return ImVec4{ _color.r(), _color.g(), _color.b(), _color.a() };
	}

	constexpr ImVec2 toImVec2(const Vec2& _vec)
	{
		return ImVec2{ static_cast<float>(_vec._vec[0]), static_cast<float>(_vec._vec[1]) };
	}

}

#define HMP_GUI_UTILS_CONTROLS_IMPL
#include <HMP/Gui/Utils/Controls.tpp>
#undef HMP_GUI_UTILS_CONTROLS_IMPL
