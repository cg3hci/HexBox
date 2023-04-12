#pragma once

#include <HMP/Gui/DagViewer/Layout.hpp>
#include <HMP/Dag/Node.hpp>

namespace HMP::Gui::DagViewer
{

	Layout createLayout(const Dag::Node& _dag);

}