#include <HMP/Dag/Utils.hpp>

#include <cpputils/unreachable.hpp>
#include <HMP/Dag/Delete.hpp>
#include <HMP/Dag/Refine.hpp>
#include <HMP/Dag/Extrude.hpp>
#include <HMP/Dag/Element.hpp>
#include <deque>
#include <vector>
#include <unordered_set>
#include <unordered_map>

namespace HMP::Dag::Utils
{

	std::vector<Node*> descendants(Node& _node, std::function<bool(const Node&)> _branchSelector)
	{
		std::vector<Node*> nodes{};
		std::unordered_set<Node*> visited{};
		std::deque<Node*> toVisit{};
		if (_branchSelector(_node))
		{
			nodes.push_back(&_node);
			visited.emplace(&_node);
			toVisit.push_back(&_node);
		}
		while (!toVisit.empty())
		{
			Node& node{ *toVisit.front() };
			toVisit.pop_front();
			for (Node& child : node.children)
			{
				if (_branchSelector(child) && visited.emplace(&child).second)
				{
					nodes.push_back(&child);
					toVisit.push_back(&child);
				}
			}
		}
		return nodes;
	}

	std::vector<const Node*> descendants(const Node& _node, std::function<bool(const Node&)> _branchSelector)
	{
		std::vector<Node*> result{ descendants(const_cast<Node&>(_node), _branchSelector) };
		return std::vector<const Node*>{&result[0], (&result[0]) + result.size()};
	}

	void serialize(HMP::Utils::Serialization::Serializer& _serializer, const Node& _root)
	{
		const std::vector<const Node*> nodes{ descendants(_root) };
		_serializer << nodes.size();
		for (const Node* node : nodes)
		{
			_serializer << node->type;
			switch (node->type)
			{
				case Node::EType::Element:
				{
					const Element& element{ node->element() };
					for (const Id vid : element.vids)
					{
						_serializer << vid;
					}
				}
				break;
				case Node::EType::Operation:
				{
					const Operation& operation{ node->operation() };
					_serializer << operation.primitive;
					switch (operation.primitive)
					{
						case Operation::EPrimitive::Extrude:
						{
							const Extrude& extrudeOperation{ static_cast<const Extrude&>(operation) };
							_serializer
								<< extrudeOperation.source
								<< extrudeOperation.firstVi
								<< extrudeOperation.clockwise
								<< extrudeOperation.fis.size();
							for (const I fi : extrudeOperation.fis)
							{
								_serializer << fi;
							}
						}
						break;
						case Operation::EPrimitive::Refine:
						{
							const Refine& refineOperation{ static_cast<const Refine&>(operation) };
							_serializer
								<< refineOperation.forwardFi
								<< refineOperation.firstVi
								<< refineOperation.scheme
								<< refineOperation.surfVids.size();
							for (const Id vid : refineOperation.surfVids)
							{
								_serializer << vid;
							}
						}
						break;
						default:
							break;
					}
				}
				break;
			}
		}
		{
			std::unordered_map<const Node*, I> nodeMap{};
			{
				nodeMap.reserve(nodes.size());
				Id i{ 0 };
				for (const Node* node : nodes)
				{
					nodeMap.emplace(node, i++);
				}
			}
			for (const Node* node : nodes)
			{
				_serializer << node->parents.size();
				for (const Node& parent : node->parents)
				{
					_serializer << nodeMap.at(&parent);
				}
			}
		}
	}

	Node& deserialize(HMP::Utils::Serialization::Deserializer& _deserializer)
	{
		std::vector<Node*> nodes{};
		I nodesCount{};
		_deserializer >> nodesCount;
		nodes.reserve(nodesCount);
		for (I i{ 0 }; i < nodesCount; i++)
		{
			Node::EType nodeType;
			_deserializer >> nodeType;
			switch (nodeType)
			{
				case Node::EType::Element:
				{
					Element& element{ *new Element{} };
					for (Id& vid : element.vids)
					{
						_deserializer >> vid;
					}
					nodes.push_back(&element);
				}
				break;
				case Node::EType::Operation:
				{
					Operation::EPrimitive primitive;
					Operation* operation{};
					_deserializer >> primitive;
					switch (primitive)
					{
						case Operation::EPrimitive::Delete:
						{
							Delete& deleteOperation{ *new Delete{} };
							operation = &deleteOperation;
						}
						break;
						case Operation::EPrimitive::Extrude:
						{
							Extrude& extrudeOperation{ *new Extrude{} };
							_deserializer
								>> extrudeOperation.source
								>> extrudeOperation.firstVi
								>> extrudeOperation.clockwise;
							extrudeOperation.fis.resize(_deserializer.get<I>());
							for (I& fi : extrudeOperation.fis)
							{
								_deserializer >> fi;
							}
							operation = &extrudeOperation;
						}
						break;
						case Operation::EPrimitive::Refine:
						{
							Refine& refineOperation{ *new Refine{} };
							_deserializer
								>> refineOperation.forwardFi
								>> refineOperation.firstVi
								>> refineOperation.scheme;
							refineOperation.surfVids.resize(_deserializer.get<I>());
							for (Id& vid : refineOperation.surfVids)
							{
								_deserializer >> vid;
							}
							operation = &refineOperation;
						}
						break;
					}
					nodes.push_back(operation);
				}
				break;
			}
		}
		for (Node* node : nodes)
		{
			I parentsCount;
			_deserializer >> parentsCount;
			for (I i{ 0 }; i < parentsCount; i++)
			{
				I parentIndex;
				_deserializer >> parentIndex;
				node->parents.attach(*nodes[parentIndex]);
			}
		}
		assert(!nodes.empty());
		return *nodes[0];
	}

	Node& clone(const Node& _node)
	{
		switch (_node.type)
		{
			case Node::EType::Element:
			{
				const Element& source{ _node.element() };
				Element& clone{ *new Element{} };
				clone.vids = source.vids;
				clone.pid = source.pid;
				return clone;
			}
			case Node::EType::Operation:
			{
				switch (_node.operation().primitive)
				{
					case Operation::EPrimitive::Delete:
					{
						Delete& clone{ *new Delete{} };
						return clone;
					}
					case Operation::EPrimitive::Extrude:
					{
						const Extrude& source{ static_cast<const Extrude&>(_node) };
						Extrude& clone{ *new Extrude{} };
						clone.firstVi = source.firstVi;
						clone.fis = source.fis;
						clone.source = source.source;
						clone.clockwise = source.clockwise;
						return clone;
					}
					case Operation::EPrimitive::Refine:
					{
						const Refine& source{ static_cast<const Refine&>(_node) };
						Refine& clone{ *new Refine{} };
						clone.scheme = source.scheme;
						clone.forwardFi = source.forwardFi;
						clone.firstVi = source.firstVi;
						clone.surfVids = source.surfVids;
						return clone;
					}
					default:
						cpputils::unreachable();
				}
			}
			default:
				cpputils::unreachable();
		}
	}

}