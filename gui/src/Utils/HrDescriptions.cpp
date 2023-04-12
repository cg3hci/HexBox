#include <HMP/Gui/Utils/HrDescriptions.hpp>

#include <cpputils/unreachable.hpp>
#include <sstream>
#include <iomanip>
#include <cpputils/range/of.hpp>
#include <cpputils/range/enumerate.hpp>
#include <cassert>

namespace HMP::Gui::Utils::HrDescriptions
{

	std::string name(const HMP::Dag::Node& _node, DagNamer& _dagNamer)
	{
		switch (_node.type)
		{
			case HMP::Dag::Node::EType::Element:
				return _dagNamer(&_node);
			case HMP::Dag::Node::EType::Operation:
				switch (_node.operation().primitive)
				{
					case HMP::Dag::Operation::EPrimitive::Delete:
						return "D-" + _dagNamer(&_node);
					case HMP::Dag::Operation::EPrimitive::Extrude:
						return "E-" + _dagNamer(&_node);
					case HMP::Dag::Operation::EPrimitive::Refine:
						return "R-" + _dagNamer(&_node);
					default:
						cpputils::unreachable();
				}
			default:
				return "?-" + _dagNamer(&_node);
		}
	}

	std::string describe(Refinement::EScheme _scheme)
	{
		switch (_scheme)
		{
			case Refinement::EScheme::Inset:
				return "Inset";
			case Refinement::EScheme::Subdivide3x3:
				return "Subdivide3x3";
			case Refinement::EScheme::AdapterEdgeSubdivide3x3:
				return "AdapterEdgeSubdivide3x3";
			case Refinement::EScheme::AdapterFaceSubdivide3x3:
				return "AdapterFaceSubdivide3x3";
			case Refinement::EScheme::Subdivide2x2:
				return "Subdivide2x2";
			case Refinement::EScheme::PlaneSplit:
				return "PlaneSplit";
			default:
				return "Unknown";
		}
	}

	std::string describe(const Vec& _vec)
	{
		std::ostringstream stream{};
		stream
			<< "{"
			<< std::fixed << std::setprecision(3)
			<< _vec.x() << ","
			<< _vec.y() << ","
			<< _vec.z()
			<< "}";
		return stream.str();
	}

	std::string describe(const Mat4& _mat)
	{
		std::ostringstream stream{};
		stream
			<< std::fixed << std::setprecision(3)
			<< "{";
		for (Id r{ 0 }; r < 4; r++)
		{
			if (r)
			{
				stream << ",";
			}
			stream << "{";
			for (Id c{ 0 }; c < 4; c++)
			{
				if (c)
				{
					stream << ",";
				}
				stream << _mat(r, c);
			}
			stream << "}";
		}
		stream << "}";
		return stream.str();
	}

	std::string describe(const std::vector<I>& _is)
	{
		std::ostringstream stream{};
		stream << "[";
		if (!_is.empty())
		{
			stream << _is[0];
			for (I i{ 1 }; i < _is.size(); i++)
			{
				stream << "," << _is[i];
			}
		}
		stream << "]";
		return stream.str();
	}

	std::string describe(const std::vector<Id>& _ids)
	{
		std::ostringstream stream{};
		stream << "[";
		if (!_ids.empty())
		{
			stream << _ids[0];
			for (I i{ 1 }; i < _ids.size(); i++)
			{
				stream << "," << _ids[i];
			}
		}
		stream << "]";
		return stream.str();
	}

	std::string describe(const std::vector<bool>& _flags)
	{
		std::ostringstream stream{};
		stream << "[";
		if (!_flags.empty())
		{
			stream << (_flags[0] ? '+' : '-');
			for (I i{ 1 }; i < _flags.size(); i++)
			{
				stream << "," << (_flags[i] ? '+' : '-');
			}
		}
		stream << "]";
		return stream.str();
	}

	std::string describe(const HMP::Dag::Delete& _operation, const HMP::Dag::Element& _element, DagNamer& _dagNamer)
	{
		std::ostringstream stream{};
		stream
			<< "Delete"
			<< " " << name(_element, _dagNamer)
			<< " (" << name(_operation, _dagNamer) << ")";
		return stream.str();
	}

	std::string describe(Dag::Extrude::ESource _source)
	{
		switch (_source)
		{
			case HMP::Dag::Extrude::ESource::Face:
				return "Face";
			case HMP::Dag::Extrude::ESource::Edge:
				return "Edge";
			case HMP::Dag::Extrude::ESource::Vertex:
				return "Vertex";
			default:
				cpputils::unreachable();
		}
	}

	std::string describe(const HMP::Dag::Extrude& _operation, const cpputils::collections::FixedVector<const HMP::Dag::Element*, 3>& _elements, DagNamer& _dagNamer)
	{
		std::ostringstream stream{};
		stream << "Extrude ";
		for (const auto& [i, element] : cpputils::range::enumerate(_elements))
		{
			if (i)
			{
				stream << ", ";
			}
			stream << name(*element, _dagNamer);
		}
		stream
			<< " (" << describe(_operation.source) << ")"
			<< " towards " << describe(cpputils::range::of(_operation.fis).toVector())
			<< " (" << _operation.firstVi << " " << (_operation.clockwise ? "CW" : "CCW") << ")"
			<< " into " << name(_operation.children.single(), _dagNamer)
			<< " (" << name(_operation, _dagNamer) << ")";
		return stream.str();
	}

	std::string describe(const HMP::Dag::Refine& _operation, const HMP::Dag::Element& _element, DagNamer& _dagNamer)
	{
		std::ostringstream stream{};
		stream
			<< "Refine"
			<< " " << name(_element, _dagNamer)
			<< " with scheme " << describe(_operation.scheme)
			<< " towards " << describe(std::vector<I>{ _operation.forwardFi, _operation.firstVi })
			<< " (" << name(_operation, _dagNamer) << ")";
		return stream.str();
	}

	std::string describe(const HMP::Dag::Delete& _operation, DagNamer& _dagNamer)
	{
		return describe(_operation, _operation.parents.single(), _dagNamer);
	}

	std::string describe(const HMP::Dag::Extrude& _operation, DagNamer& _dagNamer)
	{
		return describe(_operation, _operation.parents.address().toFixedVector<3>(), _dagNamer);
	}

	std::string describe(const HMP::Dag::Refine& _operation, DagNamer& _dagNamer)
	{
		return describe(_operation, _operation.parents.single(), _dagNamer);
	}

	std::string describe(const Actions::RefineSome& _action)
	{
		std::ostringstream stream{};
		stream
			<< "Refine"
			<< " " << _action.operations().size() << " elements"
			<< " with scheme " << describe(HMP::Refinement::EScheme::Subdivide3x3);
		return stream.str();
	}

	std::string describe(const Actions::SplitPlane& _action, DagNamer& _dagNamer)
	{
		std::ostringstream stream{};
		stream
			<< "Split plane"
			<< " from" << _action.eid()
			<< " (" << _action.operations().size() << " operations)";
		return stream.str();
	}

	std::string describe(const Actions::DeleteSome& _action)
	{
		std::ostringstream stream{};
		stream
			<< "Delete"
			<< " " << _action.operations().size() << " elements";
		return stream.str();
	}

	std::string describe(const Actions::Delete& _action, DagNamer& _dagNamer)
	{
		return describe(_action.operation(), _action.element(), _dagNamer);
	}

	std::string describe(const Actions::Extrude& _action, DagNamer& _dagNamer)
	{
		return describe(_action.operation(), _action.elements().address().toFixedVector<3>(), _dagNamer);
	}

	std::string describe(const Actions::FitCircle& _action)
	{
		std::ostringstream stream{};
		stream
			<< "Fit circle in"
			<< " " << _action.vids().size() << " verts";
		return stream.str();
	}

	std::string describe(const Actions::Pad& _action)
	{
		std::ostringstream stream{};
		stream
			<< "Pad"
			<< " with length " << _action.length()
			<< " (" << _action.operations().size() << " operations)";
		return stream.str();
	}

	std::string describe(const Actions::Root& _action, DagNamer& _dagNamer)
	{
		std::ostringstream stream{};
		stream
			<< "Root"
			<< " " << name(_action.newRoot(), _dagNamer);
		return stream.str();
	}

	std::string describe(const Actions::SubdivideAll& _action)
	{
		std::ostringstream stream{};
		stream
			<< "Subdivide all"
			<< " (" << _action.operations().size() << " operations)";
		return stream.str();
	}

	std::string describe(const Actions::MakeConforming& _action, DagNamer& _dagNamer)
	{
		std::ostringstream stream{};
		stream << "Make conforming";
		stream << " (";
		const Actions::MakeConforming::Operations& operations{ _action.operations() };
		if (operations.empty())
		{
			stream << "no operations";
		}
		if (operations.size() > 5)
		{
			stream << operations.size() << " operations";
		}
		else
		{
			bool first{ true };
			for (const auto& [operation, element] : operations)
			{
				if (!first)
				{
					stream << ", ";
				}
				first = false;
				stream << name(operation, _dagNamer) << " of " << name(element, _dagNamer);
			}
		}
		stream << ")";
		return stream.str();
	}

	std::string describe(const Actions::Paste& _action, DagNamer& _dagNamer)
	{
		std::ostringstream stream{};
		stream << "Paste"
			<< " at ";
		for (const auto& [i, element] : cpputils::range::enumerate(_action.elements()))
		{
			if (i)
			{
				stream << ", ";
			}
			stream << name(element, _dagNamer);
		}
		stream
			<< " (" << describe(_action.operation().source) << ")"
			<< " towards " << describe(cpputils::range::of(_action.operation().fis).toVector())
			<< " (" << _action.operation().firstVi << " " << (_action.operation().clockwise ? "CW" : "CCW") << ")"
			<< " into " << name(_action.operation().children.single(), _dagNamer)
			<< " (" << name(_action.operation(), _dagNamer) << ")";
		return stream.str();
	}

	std::string describe(const Actions::Project& _action)
	{
		std::ostringstream stream{};
		stream
			<< "Project"
			<< " ("
			<< _action.options().iterations
			<< " iterations)";
		return stream.str();
	}

	std::string describe(const Actions::Refine& _action, DagNamer& _dagNamer)
	{
		std::ostringstream stream{};
		stream
			<< describe(_action.operation(), _action.element(), _dagNamer)
			<< " with depth " << _action.depth();
		return stream.str();
	}

	std::string describe(const Actions::Transform& _action)
	{
		std::ostringstream stream{};
		stream << "Transform ";
		if (_action.vids())
		{
			stream << _action.vids()->size();
		}
		else
		{
			stream << "all";
		}
		stream << " verts " << describe(_action.transform());
		return stream.str();
	}

	std::string describe(const Actions::Smooth& _action)
	{
		std::ostringstream stream{};
		stream
			<< "Smooth "
			<< "(" << _action.surfaceIterations() << " surf, " << _action.internalIterations() << " int)";
		return stream.str();
	}

	std::string describe(const Commander::Action& _action, DagNamer& _dagNamer)
	{
		if (const Actions::Delete* action{ dynamic_cast<const Actions::Delete*>(&_action) }; action)
		{
			return describe(*action, _dagNamer);
		}
		if (const Actions::Extrude* action{ dynamic_cast<const Actions::Extrude*>(&_action) }; action)
		{
			return describe(*action, _dagNamer);
		}
		if (const Actions::Root* action{ dynamic_cast<const Actions::Root*>(&_action) }; action)
		{
			return describe(*action, _dagNamer);
		}
		if (const Actions::MakeConforming* action{ dynamic_cast<const Actions::MakeConforming*>(&_action) }; action)
		{
			return describe(*action, _dagNamer);
		}
		if (const Actions::Paste* action{ dynamic_cast<const Actions::Paste*>(&_action) }; action)
		{
			return describe(*action, _dagNamer);
		}
		if (const Actions::Project* action{ dynamic_cast<const Actions::Project*>(&_action) }; action)
		{
			return describe(*action);
		}
		if (const Actions::Refine* action{ dynamic_cast<const Actions::Refine*>(&_action) }; action)
		{
			return describe(*action, _dagNamer);
		}
		if (const Actions::Transform* action{ dynamic_cast<const Actions::Transform*>(&_action) }; action)
		{
			return describe(*action);
		}
		if (const Actions::Pad* action{ dynamic_cast<const Actions::Pad*>(&_action) }; action)
		{
			return describe(*action);
		}
		if (const Actions::SubdivideAll* action{ dynamic_cast<const Actions::SubdivideAll*>(&_action) }; action)
		{
			return describe(*action);
		}
		if (const Actions::Smooth* action{ dynamic_cast<const Actions::Smooth*>(&_action) }; action)
		{
			return describe(*action);
		}
		if (const Actions::RefineSome* action{ dynamic_cast<const Actions::RefineSome*>(&_action) }; action)
		{
			return describe(*action);
		}
		if (const Actions::FitCircle* action{ dynamic_cast<const Actions::FitCircle*>(&_action) }; action)
		{
			return describe(*action);
		}
		if (const Actions::DeleteSome* action{ dynamic_cast<const Actions::DeleteSome*>(&_action) }; action)
		{
			return describe(*action);
		}
		if (const Actions::SplitPlane* action{ dynamic_cast<const Actions::SplitPlane*>(&_action) }; action)
		{
			return describe(*action, _dagNamer);
		}
		return "Unknown action";
	}

}