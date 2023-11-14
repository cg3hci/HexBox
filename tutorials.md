# Tutorials

On this page you will find the description of the main features of HexBox.

## Refinement


### Global Refinement
Global refinement of hex meshes involves subdividing each hexahedral element into 
smaller hexes to increase mesh density and improve solution accuracy. This process enhances the resolution of the computational
model and it is obtained by pressing Ctrl+0.
<div align="center">
  <video src="https://github.com/pusceddusimone/HexBox/assets/92328763/28b8cb7e-ed45-4d71-a3eb-199b7b306ce3"/>
</div>

### Local Refinement
Local refinement in hexahedral (hex) meshes is a process used to enhance
the mesh quality and accuracy in specific regions of interest. This technique
selectively subdivides hexahedral elements and it is obtained by pressing H
while hovering on the element to refine.
<div align="center">
  <video src="https://github.com/pusceddusimone/HexBox/assets/92328763/1cc91973-83ec-4aa4-b7f0-f7369f0fac8e"/>
</div>


### Make Conforming
After a local refinement, it is possible to restore mesh conformity by pressing Q.
<div align="center">
  <video src="https://github.com/pusceddusimone/HexBox/assets/92328763/62d25a34-333f-4d80-9519-54047e0b0c94"/>
</div>
![make conforming](https://github.com/pusceddusimone/HexBox/assets/92328763/c016b0b3-1a46-4ec5-b0e8-e1130f64001e)


## Extrusion

### Single Face Extrusion
Extrusion of a single face can be achieved by hovering over
the element you want to extrude and pressing E.
<div align="center">
  <video src="https://github.com/pusceddusimone/HexBox/assets/92328763/e6687db8-907b-43ca-93a4-5a83b3d8885e"/>
</div>


### Extrusion of two adjacent faces in a concave configuration
Extruding two adjacent faces in a concave configuration results
in two new elements that are not adjacent to each other; this result
can be obtained by selecting two faces in a concave configuration by
using Ctrl+MouseLeft and then by pressing E.
<div align="center">
  <video src="https://github.com/pusceddusimone/HexBox/assets/92328763/4ddd0952-e752-4461-8d3a-43d61cd2f4a2"/>
</div>

### Extrusion of two adjacent faces in a convex configuration
Extruding two adjacent faces in a concave configuration results
in one new element adjacent to both faces; this result
can be obtained by selecting two faces in a convex configuration by
using Ctrl+MouseLeft and then by pressing E.
<div align="center">
  <video src="https://github.com/pusceddusimone/HexBox/assets/92328763/6d52676b-3aec-4ff1-b7ad-f618079d10e2"/>
</div>

### Extrusion of three adjacent faces in a convex configuration
Extruding three adjacent faces in a concave configuration results
in three new elements that are not adjacent to each other; this result
can be obtained by selecting three faces in a concave configuration by
using Ctrl+MouseLeft and then by pressing E.
<div align="center">
  <video src="https://github.com/pusceddusimone/HexBox/assets/92328763/16c0f930-bbbe-4de4-b9d0-eced8c1c19d9"/>
</div>
    
### Extrusion of three adjacent faces in a convex configuration
Extruding three adjacent faces in a concave configuration results
in one new element adjacent to all three faces; this result
can be obtained by selecting two faces in a convex configuration by
using Ctrl+MouseLeft and then by pressing E.
<div align="center">
  <video src="https://github.com/pusceddusimone/HexBox/assets/92328763/5056176f-ac88-456c-9e92-3b20f454da17"/>
</div>



### Extrusion of n adjacent faces in the plane
Extruding N adjacent faces in the plane will generate N
new elements that will be adjacent to each other provided
their source faces are adjacent; this result can be obtained by selecting N
adjacent faces in the plane by using Ctrl+MouseLeft and then by pressing E.
<div align="center">
  <video src="https://github.com/pusceddusimone/HexBox/assets/92328763/82a85149-c651-458f-9683-badeee4b167c"/>
</div>

