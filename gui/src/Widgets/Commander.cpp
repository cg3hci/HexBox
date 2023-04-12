#include <HMP/Gui/Widgets/Commander.hpp>

#include <HMP/Meshing/types.hpp>
#include <HMP/Gui/Utils/Controls.hpp>
#include <HMP/Gui/Utils/HrDescriptions.hpp>
#include <imgui.h>
#include <string>
#include <HMP/Gui/Widgets/VertEdit.hpp>
#include <HMP/Gui/themer.hpp>
#include <HMP/Gui/App.hpp>

namespace HMP::Gui::Widgets
{

	Commander::Commander(): SidebarWidget{ "Commander" }
	{}

	bool Commander::keyPressed(const cinolib::KeyBinding& _key)
	{
		if (_key == c_kbUndo)
		{
			app().undo();
		}
		else if (_key == c_kbRedo)
		{
			app().redo();
		}
		else
		{
			return false;
		}
		return true;
	}

	void Commander::printUsage() const
	{
		cinolib::print_binding(c_kbUndo.name(), "undo");
		cinolib::print_binding(c_kbRedo.name(), "redo");
	}

	void Commander::drawSidebar()
	{
		constexpr auto actionsControl{ [](HMP::Commander::Stack& _stack, const std::string& _name) {
			int limit{ static_cast<int>(_stack.limit()) };
			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			ImGui::SliderInt((_name + " limit").c_str(), &limit, 0, 100, "Max %d actions", ImGuiSliderFlags_AlwaysClamp);
			_stack.limit(static_cast<I>(limit));
			ImGui::TableNextColumn();
			if (Utils::Controls::disabledButton((std::string{ "Clear " } + std::to_string(_stack.size()) + " actions").c_str(), !_stack.empty()))
			{
				_stack.clear();
			}
		} };
		ImGui::BeginTable("stacks", 2, ImGuiTableFlags_RowBg);
		ImGui::TableSetupColumn("size", ImGuiTableColumnFlags_WidthStretch);
		ImGui::TableSetupColumn("clear", ImGuiTableColumnFlags_WidthFixed);
		actionsControl(app().commander.applied(), "Undo");
		actionsControl(app().commander.unapplied(), "Redo");
		ImGui::EndTable();
		ImGui::Spacing();

		ImGui::Text("History");
		ImGui::BeginChild("history", { ImGui::GetContentRegionAvail().x, 130 * themer->sbScale }, true);
		if (app().vertEditWidget.pendingAction())
		{
			ImGui::TextColored(themer->sbWarn, "Pending vertex edit action on %d vertices", static_cast<int>(app().vertEditWidget.vids().size()));
		}

		for (const auto& action : app().commander.unapplied().reverse())
		{
			ImGui::TextColored(themer->sbErr, "%s", Utils::HrDescriptions::describe(action, app().dagNamer).c_str());
		}

		for (const HMP::Commander::Action& action : app().commander.applied())
		{
			ImGui::TextColored(themer->sbOk, "%s", Utils::HrDescriptions::describe(action, app().dagNamer).c_str());
		}
		ImGui::EndChild();
	}

}