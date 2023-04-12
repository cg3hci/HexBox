#pragma once

#include <HMP/Commander.hpp>
#include <HMP/Dag/NodeHandle.hpp>
#include <HMP/Dag/Extrude.hpp>
#include <cpputils/range/of.hpp>
#include <array>
#include <utility>

namespace HMP::Actions
{

    class Pad final: public Commander::Action
    {

    private:

        static std::pair<const Dag::Element&, const Dag::Extrude&> dereferenceOperation(const std::pair<Dag::Element* const, const Dag::NodeHandle<Dag::Extrude>>& _pair);

        const Real m_length;
        const I m_smoothIterations;
        const Real m_smoothSurfVertWeight;
        const Real m_cornerShrinkFactor;
        std::vector<std::pair<Dag::Element* const, const Dag::NodeHandle<Dag::Extrude>>> m_operations;
        std::vector<Vec> m_otherVerts;
        std::vector<Vec> m_newVerts;
        Meshing::Mesher::State m_oldState;
        bool m_prepared;

        void swapVerts();
        void apply() override;
        void unapply() override;

    public:

        using Operations = decltype(cpputils::range::ofc(m_operations).map(&dereferenceOperation));

        Pad(Real _length, I _smoothIterations = 1, Real _smoothSurfVertWeight = 1.0, Real _cornerShrinkFactor = 0.5);

        const Real length() const;

        const I smoothIterations() const;

        const Real smoothSurfVertWeight() const;

        const Real cornerShrinkFactor() const;

        Operations operations() const;

    };

}
