#pragma once

#include <HMP/Meshing/types.hpp>
#include <HMP/Refinement/Schemes.hpp>
#include <HMP/Meshing/Mesher.hpp>
#include <HMP/Dag/Element.hpp>
#include <HMP/Dag/Refine.hpp>
#include <vector>
#include <optional>

namespace HMP::Refinement
{

    class Sub3x3AdapterCandidate final
    {

    private:

        Dag::Element* m_element;
        std::optional<EScheme> m_scheme;
        std::vector<Id> m_adjacentEids, m_adjacentFids;
        I m_forwardFi, m_firstVi;

        void setup3x3Subdivide(const Meshing::Mesher& _mesher);

        void findRightAdapter(const Meshing::Mesher& _mesher);

    public:

        Sub3x3AdapterCandidate(Dag::Element& _element);

        Dag::Element& element() const;

        EScheme scheme() const;

        void addAdjacency(const Meshing::Mesher& _mesher, const Dag::Element& _refined, bool _edge);

        Dag::Refine& prepareAdapter() const;

    };

}