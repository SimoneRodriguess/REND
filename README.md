# REND: Procedural Scene Description Language

A domain-specific language (DSL) written in C++ that takes a scene description file (.rend) and outputs a .obj + .mtl file importable directly into Blender. Essentially a very cool procedural modelling tool wherein you describe geometry in code, get a 3D scene out.

## Pipeline
`scene.rend` → **Lexer** → **Parser** → **OBJ Codegen** → `scene.obj` + `scene.mtl`

## Example Input (scene.rend)
```cpp
camera {
    position: (0, 2, -5)
    look_at: (0, 0, 0)
    fov: 60
}

house {
    position: (0, 1.5, 0)
    size: (3, 2, 3)
    roof_height: 1.5
    color: (0.9, 0.9, 0.85)
}

tree {
    position: (2, 1.5, 2)
    trunk_height: 1.5
    canopy_radius: 1.0
    color: (0.2, 0.7, 0.3)
}
```
Output:
scene.obj: 3D geometry (vertices + faces)

scene.mtl: materials (colors per object)

Primitives Planned:
camera: position, look_at, fov

house: position, size, roof_height, color

tree: position, trunk_height, canopy_radius, color

box: position, size, color

sphere: position, radius, color
