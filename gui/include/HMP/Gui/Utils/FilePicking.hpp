#pragma once

#include <string>
#include <optional>

namespace HMP::Gui::Utils::FilePicking
{

	std::optional<std::string> open();
	
	std::optional<std::string> save(const std::string& _extension);

	std::optional<std::string> save();


}