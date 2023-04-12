#pragma once

#include <HMP/Commander.hpp>
#include <HMP/Meshing/types.hpp>
#include <vector>

namespace HMP::Actions
{

	class FitCircle final : public Commander::Action
	{

	private:

		std::vector<Id> m_vids;
		std::vector<Vec> m_otherVerts;
		bool m_prepared;

		void apply() override;
		void unapply() override;

	public:

		FitCircle(const std::vector<Id>& _vids);

		const std::vector<Id>& vids() const;

	};

}
