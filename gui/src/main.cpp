
#include <HMP/Gui/App.hpp>
#include <iostream>

int main(int _argc, char* _argv[])
{
	std::optional<std::string> file{ std::nullopt };
	if (_argc == 2)
	{
		file = _argv[1];
		std::cout << "loading file '" << *file << "'" << std::endl;
	}
	else if (_argc > 2)
	{
		std::cerr << "expected 0 or 1 argument, got " << _argc - 1 << std::endl;
		return 1;
	}
	return HMP::Gui::App::run(file);
}