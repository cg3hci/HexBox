# Issues and possible improvements
Sorted by priority:
- **\[IMPROVEMENT\]** Revert `HMP::Actions::ExtrudeUtils::apply` new vertex creation to the old method.
- **\[IMPROVEMENT\]** `HMP::Projection::jacobianAdvance` is too defensive. Try working on one vertex at a time somehow.
- **\[FEATURE\]** Add a negative weight factor for elements with multiple faces on surface in `HMP::Projection::smoothInternal`.
- **\[BUG\]** `HMP::Dag::Extrude` and `HMP::Dag::Refine` relative fis and vis are cloned incorrectly during a paste operation, so I am recalculating them from scratch in `HMP::Actions::Paste::fixAdjacencies_TEMP_NAIVE`. This implies that:
    1. The otherwise flawless "*Paste-the-same-way-to-get-the-same-result*" principle is violated (not a big deal; GUI helps a lot here);
    2. I cannot profit from `HMP::Refinement::Scheme::facesSurfVisIs` until this is fixed.
- **\[BUG\]** `HMP::Actions::Root::~Root()` leads to `HMP::Dag::Node` double free on app exit (I think the `HMP::Dag::Node` detachment system is broken).
- **\[IMPROVEMENT\]** `HMP::Refinement::Utils::apply` should use `HMP::Refinement::Scheme::facesSurfVisIs` in place of `HMP::Refinement::Utils::weldAdjacencies_TEMP_NAIVE` (how do I rotate the scheme vertices and match the adjacent faces?).
- **\[IMPROVEMENT\]** Improve `HMP::Actions::MakeConforming` either by implementing a balancing preprocessing phase, or by defining more adapter schemes.
- **\[FEATURE\]** The extrude operation could automatically determine the number of parents, or at least give the user a warning in case of unintentionally duplicate vertices.
- **\[IMPROVEMENT\]** `HMP::Actions::MakeConforming` performance can be improved a lot by keeping a queue of non-conforming refinements.
- **\[FEATURE\]** Perhaps pasting a subtree should not preserve the source size (or maybe the choice could be left to the user).
- **\[REFACTOR\]** `HMP::Meshing::Utils` is a dumpsite full of duplicated code. Keep the few essential primitives and throw everything else away.
- **\[IMPROVEMENT\]** `OGDF` is overkill for what I need. Consider replacing it with a lighter implementation of the Sugiyama layout algorithm.
- **\[IMPROVEMENT\]** The `HMP::Gui::Widgets::DirectVertEdit` scale and rotation implementation is a bit janky. (Also, a global axis-aligned translation feature would be welcome).
- **\[FEATURE\]** Add a command to select all the vertices in a subtree.
- **\[FEATURE\]** Allow the user to change the transform origin when editing vertices (or add a command to lock the origin in the current location).
- **\[FEATURE\]** Add As-Rigid-As-Possible vertex editing support.
- **\[REFACTOR\]** All the `HMP::Meshing::Actions` could be replaced with a set of more primitive actions (`MoveVert`, `ShowElement`, `AddElements`, `WeldElements` and `ActionSequence` maybe?).
- **\[IMPROVEMENT\]** The `HMP::Gui::DagViewer::Widget` view should try to remain stable on layout change.
- **\[IMPROVEMENT\]** The camera pan speed could be adjusted depending on the relative mesh position (as I do in the `HMP::Gui::Widgets::DirectVertEdit` translation operation).
