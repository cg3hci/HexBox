#pragma once

#include <HMP/Meshing/types.hpp>
#include <HMP/Meshing/Mesher.hpp>
#include <HMP/Dag/Element.hpp>
#include <HMP/Dag/Refine.hpp>
#include <HMP/Refinement/Sub3x3AdapterCandidate.hpp>
#include <unordered_map>

namespace HMP::Refinement
{

    class Sub3x3AdapterCandidateSet final
    {

    private:

        using Map = std::unordered_map<Dag::Element*, Sub3x3AdapterCandidate>;

        Map m_sub3x3Map{}; // candidates that will be refined as a new Subdivide3x3 (they need to be processed first)
        Map m_nonSub3x3Map{}; // other candidates (they must be processed only after)

        void addAdjacency(const Meshing::Mesher& _mesher, Dag::Element& _candidate, const Dag::Element& _refined, bool _edge);

    public:

        void addAdjacency(Meshing::Mesher& _mesher, Dag::Refine& _refine);

        Sub3x3AdapterCandidate pop();

        bool empty() const;

    };

}