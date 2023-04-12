#include <HMP/Gui/DagViewer/Layout.hpp>

#include <utility>
#include <algorithm>
#include <cassert>

namespace HMP::Gui::DagViewer
{

	// Layout::Node

	Layout::Node::Node(const Vec2& _center, const Dag::Node& _node)
		: m_center(_center), m_node{ &_node }
	{}

	const Vec2& Layout::Node::center() const
	{
		return m_center;
	}

	const Dag::Node& Layout::Node::node() const
	{
		return *m_node;
	}

	// Layout

	void Layout::calculateBoundingBox()
	{
		if (m_lines.empty() && m_nodes.empty())
		{
			m_topRight = m_bottomLeft = Vec2{ 0,0 };
		}
		else
		{
			if (m_nodes.empty())
			{
				m_topRight = m_bottomLeft = m_lines[0].first;
			}
			else
			{
				m_topRight = m_bottomLeft = m_nodes[0].center();
			}
			const Real halfLineThickness = m_lineThickness / 2;
			for (const std::pair<Vec2, Vec2>& line : m_lines)
			{
				expandBoundingBox(line.first, halfLineThickness);
				expandBoundingBox(line.second, halfLineThickness);
			}
			for (const Node& node : m_nodes)
			{
				expandBoundingBox(node.center(), m_nodeRadius);
			}
		}
		m_size = m_topRight - m_bottomLeft;
		m_aspectRatio = m_size.y() != 0.0 ? m_size.x() / m_size.y() : 1.0;
	}

	void Layout::expandBoundingBox(const Vec2& _center, Real _extent)
	{
		m_bottomLeft.x() = std::min(m_bottomLeft.x(), _center.x() - _extent);
		m_bottomLeft.y() = std::min(m_bottomLeft.y(), _center.y() - _extent);
		m_topRight.x() = std::max(m_topRight.x(), _center.x() + _extent);
		m_topRight.y() = std::max(m_topRight.y(), _center.y() + _extent);
	}

	Layout::Layout()
		: m_nodes{}, m_lines{}, m_lineThickness{ 0.01 }, m_nodeRadius{ 1 }
	{
		calculateBoundingBox();
	}

	Layout::Layout(const std::vector<Node>& _nodes, const std::vector<std::pair<Vec2, Vec2>>& _lines, Real _nodeRadius, Real _lineThickness)
		: m_nodes{ _nodes }, m_lines{ _lines }, m_lineThickness{ _lineThickness }, m_nodeRadius{ _nodeRadius }
	{
		assert(m_nodeRadius > 0.f);
		assert(m_lineThickness > 0.f);
		calculateBoundingBox();
	}

	Layout::Layout(std::vector<Node>&& _nodes, std::vector<std::pair<Vec2, Vec2>>&& _lines, Real _nodeRadius, Real _lineThickness)
		: m_nodes{ std::move(_nodes) }, m_lines{ std::move(_lines) }, m_lineThickness{ _lineThickness }, m_nodeRadius{ _nodeRadius }
	{
		assert(m_nodeRadius > 0.f);
		assert(m_lineThickness > 0.f);
		calculateBoundingBox();
	}

	const std::vector<std::pair<Vec2, Vec2>>& Layout::lines() const
	{
		return m_lines;
	}

	const std::vector<Layout::Node>& Layout::nodes() const
	{
		return m_nodes;
	}

	const Vec2& Layout::bottomLeft() const
	{
		return m_bottomLeft;
	}

	const Vec2& Layout::topRight() const
	{
		return m_topRight;
	}

	const Vec2& Layout::size() const
	{
		return m_size;
	}

	Real Layout::aspectRatio() const
	{
		return m_aspectRatio;
	}

	Real Layout::lineThickness() const
	{
		return m_lineThickness;
	}

	Real Layout::nodeRadius() const
	{
		return m_nodeRadius;
	}

}