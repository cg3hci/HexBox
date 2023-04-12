#pragma once

#include <utility>
#include <vector>
#include <HMP/Dag/Node.hpp>
#include <HMP/Meshing/types.hpp>

namespace HMP::Gui::DagViewer
{

	class Layout final
	{

	public:

		class Node final
		{

		private:

			Vec2 m_center{};
			const Dag::Node* m_node{};


		public:

			Node(const Vec2& _center, const Dag::Node& _node);

			const Vec2& center() const;
			const Dag::Node& node() const;

		};

	private:

		std::vector<Node> m_nodes;
		std::vector<std::pair<Vec2, Vec2>> m_lines;
		Real m_lineThickness;
		Real m_nodeRadius;
		Vec2 m_bottomLeft;
		Vec2 m_topRight;
		Vec2 m_size;
		Real m_aspectRatio;

		void calculateBoundingBox();
		void expandBoundingBox(const Vec2& _center, Real _extent);

	public:

		Layout();
		Layout(const std::vector<Node>& _nodes, const std::vector<std::pair<Vec2, Vec2>>& _lines, Real _lineThickness, Real _nodeRadius);
		Layout(std::vector<Node>&& _nodes, std::vector<std::pair<Vec2, Vec2>>&& _lines, Real _lineThickness, Real _nodeRadius);

		const std::vector<std::pair<Vec2, Vec2>>& lines() const;
		const std::vector<Node>& nodes() const;
		const Vec2& bottomLeft() const;
		const Vec2& topRight() const;
		const Vec2& size() const;
		double aspectRatio() const;
		Real lineThickness() const;
		Real nodeRadius() const;

	};

}