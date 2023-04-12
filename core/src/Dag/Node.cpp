#include <HMP/Dag/Node.hpp>

#include <cassert>
#include <cpputils/ensure.hpp>

namespace HMP::Dag
{

	I Node::s_allocatedNodeCount{};

	I Node::allocatedNodeCount()
	{
		return s_allocatedNodeCount;
	}

	Node::Node(EType _type) :
		m_parentsImpl{}, m_childrenImpl{},
		m_handles{ 0 },
		type{ _type },
		parents{
			m_parentsImpl,
			[this](auto && ..._args) { return onParentAttach(_args...); },
			[this](auto && ..._args) { return onParentDetach(_args...); },
			[this](auto && ..._args) { return onParentsDetachAll(_args...); }
		},
		children{
			m_childrenImpl,
			[this](auto && ..._args) { return onChildAttach(_args...); },
			[this](auto && ..._args) { return onChildDetach(_args...); },
			[this](auto && ..._args) { return onChildrenDetachAll(_args...); }
		}
	{
		s_allocatedNodeCount++;
	}

	void Node::deleteDangling(std::queue<Node*>& _dangling, bool _descending)
	{
		while (!_dangling.empty())
		{
			Node& node{ *_dangling.front() };
			_dangling.pop();
			assert(node.back(_descending).empty());
			for (Node& next : node.forward(_descending))
			{
				ensure(next.back(_descending).data().remove(node));
				if (next.back(_descending).empty() && !next.m_handles)
				{
					_dangling.push(&next);
				}
			}
			node.forward(_descending).data().clear();
			delete& node;
		}
	}

	bool Node::onAttach(Node& _node, bool _descending)
	{
		if (forward(_descending).data().add(_node))
		{
			ensure(_node.back(_descending).data().add(*this));
			return true;
		}
		return false;
	}

	bool Node::onDetach(Node& _node, bool _deleteDangling, bool _descending)
	{
		if (forward(_descending).data().remove(_node))
		{
			ensure(_node.back(_descending).data().remove(*this));
			if (_deleteDangling && _node.back(_descending).empty())
			{
				std::queue<Node*> dangling{};
				dangling.push(&_node);
				deleteDangling(dangling, _descending);
			}
			return true;
		}
		return false;
	}

	bool Node::onDetachAll(bool _deleteDangling, bool _descending)
	{
		std::queue<Node*> dangling{};
		const bool wasEmpty{ forward(_descending).empty() };
		for (Node& next : forward(_descending))
		{
			ensure(next.back(_descending).data().remove(*this));
			if (_deleteDangling && next.back(_descending).empty())
			{
				dangling.push(&next);
			}
		}
		forward(_descending).data().clear();
		deleteDangling(dangling, _descending);
		return !wasEmpty;
	}

	bool Node::onParentAttach(Node& _parent)
	{
		onParentAttaching(_parent);
		return onAttach(_parent, false);
	}

	bool Node::onParentDetach(Node& _parent, bool _deleteDangling)
	{
		return onDetach(_parent, _deleteDangling, false);
	}

	bool Node::onParentsDetachAll(bool _deleteDangling)
	{
		return onDetachAll(_deleteDangling, false);
	}

	bool Node::onChildAttach(Node& _child)
	{
		onChildAttaching(_child);
		return onAttach(_child, true);
	}

	bool Node::onChildDetach(Node& _child, bool _deleteDangling)
	{
		return onDetach(_child, _deleteDangling, true);
	}

	bool Node::onChildrenDetachAll(bool _deleteDangling)
	{
		return onDetachAll(_deleteDangling, true);
	}

	void Node::onParentAttaching(Node&) const {}

	void Node::onChildAttaching(Node&) const {}

	Internal::NodeSetHandle& Node::parentsHandle()
	{
		return parents.handle();
	}

	Internal::NodeSetHandle& Node::childrenHandle()
	{
		return children.handle();
	}

	Node::~Node()
	{
		parents.detachAll(false);
		children.detachAll(false);
		s_allocatedNodeCount--;
	}

	bool Node::isElement() const
	{
		return type == EType::Element;
	}

	bool Node::isOperation() const
	{
		return type == EType::Operation;
	}

	bool Node::isRoot() const
	{
		return parents.empty();
	}

	bool Node::isLeaf() const
	{
		return children.empty();
	}

	Element& Node::element()
	{
		return reinterpret_cast<Element&>(*this);
	}

	const Element& Node::element() const
	{
		return reinterpret_cast<const Element&>(*this);
	}

	Operation& Node::operation()
	{
		return reinterpret_cast<Operation&>(*this);
	}

	const Operation& Node::operation() const
	{
		return reinterpret_cast<const Operation&>(*this);
	}

	Node::Set& Node::forward(bool _descending)
	{
		return _descending ? children : parents;
	}

	const Node::Set& Node::forward(bool _descending) const
	{
		return const_cast<Node*>(this)->forward(_descending);
	}

	Node::Set& Node::back(bool _descending)
	{
		return forward(!_descending);
	}

	const Node::Set& Node::back(bool _descending) const
	{
		return const_cast<Node*>(this)->back(_descending);
	}

}