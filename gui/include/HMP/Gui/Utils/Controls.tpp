#ifndef HMP_GUI_UTILS_CONTROLS_IMPL
#error __FILE__ should not be directly included
#endif

#include <HMP/Gui/Utils/Controls.hpp>

namespace HMP::Gui::Utils::Controls
{

	template<typename TEnum> requires std::is_enum_v<TEnum>
	bool combo(const char* _label, TEnum& _value, const std::initializer_list<const char*>& _values)
	{
		int value{ static_cast<int>(_value) };
		if (ImGui::Combo(_label, &value, _values.begin(), static_cast<int>(_values.size())))
		{
			_value = static_cast<TEnum>(value);
			return true;
		}
		return false;
	}

}