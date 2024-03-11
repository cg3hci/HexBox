# Tutorials

On this page you will find the description of some of the main features of HexBox (NB: Use Cmd instead of Ctrl if you use a Mac).

## Refinement


### Global Refinement
Global refinement allows you to split each hexahedron in the mesh with a 2-refinement schema (each hexahedron is split into 8 new hexahedra) to increase the whole mesh resolution. To apply this feature, select one hexahedron and press the key combination Ctrl+0. For more details, see Section 3.1.1 in the paper.

<div align="center">
  <img src="https://github.com/pusceddusimone/HexBox/assets/92328763/ee86e023-ba84-494a-986e-4f69f3c9f753" width="70%" height="70%">
</div>


### Local Refinement
Local refinement allows you to change the resolution only in selected hexahedra, with 3-refinement schemes (each hexahedron is split into 27 new hexahedra). To apply this feature, select the desired hexahedra by using Ctrl+MouseLeft and press the key H. For more details, see Section 3.1.1 in the paper.

<div align="center">
  <img src="https://github.com/pusceddusimone/HexBox/assets/92328763/960b8440-e9fb-4e65-8d46-30be937c3a38" width="70%" height="70%">
</div>

### Face Refinement
Face refinement allows you to locally edit the singular structure of the mesh, splitting a hexahedron into 6 sub-elements so as to decompose the selected face into 5 quads, 1 central and 4 lateral. To apply this feature, select one face and press the key F. For more details, see Section 3.1.1 in the paper.

<div align="center">
  <img src="https://github.com/pusceddusimone/HexBox/assets/92328763/9d1ee636-826f-40a6-a9db-9aac5a8f68df" width="70%" height="70%">
</div>

### Make Conforming
Local refinement and face refinement introduce hanging nodes, producing a non-conforming mesh. We provide the "makeConforming" functionality to restore conformity on the mesh. To apply this feature, press the key Q. For more details, see Section 3.1.1 in the paper.

<div align="center">
  <img src="https://github.com/pusceddusimone/HexBox/assets/92328763/4385a613-4a02-420d-bd5d-540c1c8e2bf6" width="70%" height="70%">
</div>

## Extrusion

### Single Face Extrusion
The extrusion of a single face can be achieved by selecting one face and pressing the key E. This operation results in a new hexahedron adjacent to the selected face. For more details, see Section 3.1.2 in the paper.

<div align="center">
  <img src="https://github.com/pusceddusimone/HexBox/assets/92328763/b8bae51c-3840-40c3-afcd-639b9cd2a7d5" width="70%" height="70%">
</div>

### Extrusion of two adjacent faces in a convex configuration
Extruding two adjacent faces in a convex configuration (meaning with a dihedral angle between them > $\pi$) results in two new hexahedra, each of them adjacent to one of the selected faces. This result can be obtained by selecting two faces in a convex configuration by using Ctrl+MouseLeft and then by pressing the key E. For more details, see Section 3.1.2 in the paper.

<div align="center">
  <img src="https://github.com/pusceddusimone/HexBox/assets/92328763/ebf39198-5857-484f-8774-831bc9fa1a02" width="70%" height="70%">
</div>

### Extrusion of two adjacent faces in a concave configuration
Extruding two adjacent faces in a concave configuration (meaning with a dihedral angle between them < $\pi$) results in a single new hexahedron adjacent to both selected faces. This result can be obtained by selecting two faces in a concave configuration by using Ctrl+MouseLeft and then by pressing the key E. For more details, see Section 3.1.2 in the paper.
<div align="center">
  <img src="https://github.com/pusceddusimone/HexBox/assets/92328763/0acfa31e-3342-46aa-8fe0-76c30f9dd0af" width="70%" height="70%">
</div>

### Extrusion of three adjacent faces in a convex configuration
Extruding three adjacent faces in a convex configuration results in three new hexahedra, each of them adjacent to one of the selected faces. This result can be obtained by selecting three faces in a convex configuration by using Ctrl+MouseLeft and then by pressing the key E. For more details, see Section 3.1.2 in the paper.

<div align="center">
  <img src="https://github.com/pusceddusimone/HexBox/assets/92328763/99f59a7e-6dad-4cd2-9c28-a1994f12c2f1" width="70%" height="70%">
</div>

### Extrusion of three adjacent faces in a concave configuration
Extruding three adjacent faces in a concave configuration results in a single new hexahedron adjacent to all the selected faces. This result
can be obtained by selecting three faces in a concave configuration by using Ctrl+MouseLeft and then by pressing the key E. For more details, see Section 3.1.2 in the paper.

<div align="center">
  <img src="https://github.com/pusceddusimone/HexBox/assets/92328763/60943502-b1e8-4334-b0e5-fa525143fab9" width="70%" height="70%">
</div>

### Extrusion of n adjacent faces in the plane
Extruding N adjacent faces in an almost flat region will generate N
new elements adjacent to each other and each of them adjacent to a selected face. This result can be obtained by selecting N adjacent faces by using Ctrl+MouseLeft and then by pressing the key E. For more details, see Section 3.1.2 in the paper.

<div align="center">
  <img src="https://github.com/pusceddusimone/HexBox/assets/92328763/803f9b44-babf-4487-81fc-c756104ebbcb" width="70%" height="70%">
</div>
