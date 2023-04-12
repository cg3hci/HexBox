#pragma once

#include <HMP/Commander.hpp>
#include <HMP/Dag/NodeHandle.hpp>
#include <HMP/Dag/Refine.hpp>
#include <cpputils/range/of.hpp>
#include <utility>

namespace HMP::Actions
{

    class SubdivideAll final: public Commander::Action
    {

    private:

        static std::pair<const Dag::Element&, const Dag::Refine&> dereferenceOperation(const std::pair<Dag::Element* const, const Dag::NodeHandle<Dag::Refine>>& _pair);

        std::vector<std::pair<Dag::Element* const, const Dag::NodeHandle<Dag::Refine>>> m_operations;
        Meshing::Mesher::State m_oldState;
        std::vector<Vec> m_newVerts;
        bool m_prepared;

        void apply() override;
        void unapply() override;

    public:

        using Operations = decltype(cpputils::range::ofc(m_operations).map(&dereferenceOperation));

        SubdivideAll();

        Operations operations() const;

    };

}
