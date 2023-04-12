#pragma once

#include <HMP/Commander.hpp>
#include <HMP/Meshing/types.hpp>
#include <utility>
#include <vector>

namespace HMP::Actions
{

    class Smooth final: public Commander::Action
    {

    private:

        const I m_surfIterations, m_internalIterations;
        const Real m_surfVertWeight;
        std::vector<Vec> m_otherVerts;
        bool m_prepared;

        void apply() override;
        void unapply() override;

    public:

        Smooth(I _surfaceIterations, I _internalIterations, Real _surfVertWeight = 1.0);

        const I surfaceIterations() const;
        
        const I internalIterations() const;

        const Real surfVertWeight() const;

    };

}
