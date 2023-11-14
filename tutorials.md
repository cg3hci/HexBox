# Tutorials

On this page you will find the description of the main features of HexBox.

## Refinement


### Global Refinement
Global refinement of hex meshes involves subdividing each hexahedral element into 
smaller hexes to increase mesh density and improve solution accuracy. This process enhances the resolution of the computational
model and it is obtained by pressing Ctrl+0.
<div align="center">
  <img src="https://github.com/pusceddusimone/HexBox/assets/92328763/ee86e023-ba84-494a-986e-4f69f3c9f753" width="70%" height="70%">
</div>

### Local Refinement
Local refinement in hexahedral (hex) meshes is a process used to enhance
the mesh quality and accuracy in specific regions of interest. This technique
selectively subdivides hexahedral elements and it is obtained by pressing H
while hovering on the element to refine.
<div align="center">
  <img src="https://github.com/pusceddusimone/HexBox/assets/92328763/960b8440-e9fb-4e65-8d46-30be937c3a38" width="70%" height="70%">
</div>

### Make Conforming
After a local refinement, it is possible to restore mesh conformity by pressing Q.
<div align="center">
  <img src="https://github.com/pusceddusimone/HexBox/assets/92328763/4385a613-4a02-420d-bd5d-540c1c8e2bf6" width="70%" height="70%">
</div>

## Extrusion

### Single Face Extrusion
Extrusion of a single face can be achieved by hovering over
the element you want to extrude and pressing E.
<div align="center">
  <img src="https://github.com/pusceddusimone/HexBox/assets/92328763/3a4edde4-85b8-49e9-8179-43aa1a54d87f" width="70%" height="70%">
</div>

### Extrusion of two adjacent faces in a concave configuration
Extruding two adjacent faces in a concave configuration results
in two new elements that are not adjacent to each other; this result
can be obtained by selecting two faces in a concave configuration by
using Ctrl+MouseLeft and then by pressing E.
<div align="center">
  <img src="https://github.com/pusceddusimone/HexBox/assets/92328763/ebf39198-5857-484f-8774-831bc9fa1a02" width="70%" height="70%">
</div>

### Extrusion of two adjacent faces in a convex configuration
Extruding two adjacent faces in a convex configuration results
in one new element adjacent to both faces; this result
can be obtained by selecting two faces in a convex configuration by
using Ctrl+MouseLeft and then by pressing E.
<div align="center">
  <img src="https://github.com/pusceddusimone/HexBox/assets/92328763/caf3298f-d3f9-44aa-bd92-0daae39d7523" width="70%" height="70%">
</div>

### Extrusion of three adjacent faces in a concave configuration
Extruding three adjacent faces in a concave configuration results
in three new elements that are not adjacent to each other; this result
can be obtained by selecting three faces in a concave configuration by
using Ctrl+MouseLeft and then by pressing E.
<div align="center">
  <img src="https://github.com/pusceddusimone/HexBox/assets/92328763/3d67bef8-103e-4648-9b82-9496f735334b" width="70%" height="70%">
</div>

### Extrusion of three adjacent faces in a convex configuration
Extruding three adjacent faces in a concave configuration results
in one new element adjacent to all three faces; this result
can be obtained by selecting two faces in a convex configuration by
using Ctrl+MouseLeft and then by pressing E.
<div align="center">
  <img src="https://github.com/pusceddusimone/HexBox/assets/92328763/04b76978-e3b8-4119-9a26-b8909a8fd2a9" width="70%" height="70%">
</div>


### Extrusion of n adjacent faces in the plane
Extruding N adjacent faces in the plane will generate N
new elements that will be adjacent to each other provided
their source faces are adjacent; this result can be obtained by selecting N
adjacent faces in the plane by using Ctrl+MouseLeft and then by pressing E.
<div align="center">
  <img src="https://github.com/pusceddusimone/HexBox/assets/92328763/803f9b44-babf-4487-81fc-c756104ebbcb" width="70%" height="70%">
</div>
