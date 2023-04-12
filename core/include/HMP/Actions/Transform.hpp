#pragma once

#include <HMP/Meshing/types.hpp>
#include <HMP/Commander.hpp>
#include <HMP/Dag/Element.hpp>
#include <vector>
#include <utility>
#include <optional>

namespace HMP::Actions
{

	class Transform final: public Commander::Action
	{

	private:

		const Mat4 m_transform;
		const std::optional<std::vector<Id>> m_vids;
		std::vector<Vec> m_otherVerts;
		bool m_prepared;

		void apply() override;
		void unapply() override;

	public:

		Transform(const Mat4& _transform, const std::optional<std::vector<Id>>& _vids = std::nullopt);

		const Mat4& transform() const;

		const std::optional<std::vector<Id>>& vids() const;

	};

}
