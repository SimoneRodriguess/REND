#include "codegen.h"
#include <fstream>
#include <stdexcept>
#include <cstdlib>
#include <cmath>
#include <vector>

static float resolveScalar(const ScalarVal& s) {
    if (!s.is_random) return s.value;
    float t = (float)rand() / RAND_MAX;
    return s.lo + t * (s.hi - s.lo);
}

struct Vec3 { float x, y, z; };

static Vec3 resolveVec3(const Vec3Val& v) {
    return { resolveScalar(v.x), resolveScalar(v.y), resolveScalar(v.z) };
}

static float getProp(const SceneObject& obj, const std::string& name, float def) {
    for (auto& p : obj.props)
        if (p.name == name && p.kind == PropKind::SCALAR)
            return resolveScalar(p.scalar);
    return def;
}

static Vec3 getVec3Prop(const SceneObject& obj, const std::string& name, Vec3 def) {
    for (auto& p : obj.props)
        if (p.name == name && p.kind == PropKind::VEC3)
            return resolveVec3(p.vec3);
    return def;
}

struct OBJWriter {
    std::ofstream obj, mtl;
    int vertOffset = 1;
    int matIndex   = 0;

    OBJWriter(const std::string& objPath, const std::string& mtlPath) {
        obj.open(objPath);
        mtl.open(mtlPath);
        if (!obj || !mtl)
            throw std::runtime_error("cannot open output files");
        std::string mtlName = mtlPath.substr(mtlPath.find_last_of("/\\") + 1);
        obj << "mtllib " << mtlName << "\n\n";
    }

    std::string newMat(Vec3 color) {
        std::string name = "mat" + std::to_string(matIndex++);
        mtl << "newmtl " << name << "\n";
        mtl << "Kd " << color.x << " " << color.y << " " << color.z << "\n";
        mtl << "Ka 0.1 0.1 0.1\n";
        mtl << "Ks 0.0 0.0 0.0\n\n";
        return name;
    }

    void writeBox(const std::string& name, Vec3 pos, Vec3 size, Vec3 color) {
        float hw = size.x / 2, hh = size.y / 2, hd = size.z / 2;
        float x = pos.x, y = pos.y, z = pos.z;

        std::string mat = newMat(color);
        obj << "o " << name << "\n";
        obj << "usemtl " << mat << "\n";

        float verts[8][3] = {
            {x-hw, y-hh, z+hd}, {x+hw, y-hh, z+hd},
            {x+hw, y+hh, z+hd}, {x-hw, y+hh, z+hd},
            {x-hw, y-hh, z-hd}, {x+hw, y-hh, z-hd},
            {x+hw, y+hh, z-hd}, {x-hw, y+hh, z-hd},
        };
        for (auto& v : verts)
            obj << "v " << v[0] << " " << v[1] << " " << v[2] << "\n";

        int o = vertOffset;
        obj << "f " << o+3 << " " << o+2 << " " << o+1 << " " << o+0 << "\n";
        obj << "f " << o+4 << " " << o+5 << " " << o+6 << " " << o+7 << "\n";
        obj << "f " << o+0 << " " << o+1 << " " << o+5 << " " << o+4 << "\n";
        obj << "f " << o+2 << " " << o+3 << " " << o+7 << " " << o+6 << "\n";
        obj << "f " << o+1 << " " << o+2 << " " << o+6 << " " << o+5 << "\n";
        obj << "f " << o+3 << " " << o+0 << " " << o+4 << " " << o+7 << "\n\n";

        vertOffset += 8;
    }
};

static void genIsland(OBJWriter& w, const SceneObject& obj, int idx) {
    Vec3 size = getVec3Prop(obj, "size", {8, 1.5f, 8});
    Vec3 color = getVec3Prop(obj, "color", {0.35f, 0.68f, 0.25f});
    w.writeBox("island_" + std::to_string(idx), {0,0,0}, size, color);
}

static void genHouse(OBJWriter& w, const SceneObject& obj, int idx) {
    Vec3 pos   = getVec3Prop(obj, "position", {0, 1.5f, 0});
    Vec3 size  = getVec3Prop(obj, "size",     {3, 2, 3});
    Vec3 color = getVec3Prop(obj, "color",    {0.9f, 0.9f, 0.85f});
    w.writeBox("house_" + std::to_string(idx), pos, size, color);
}

static void genTree(OBJWriter& w, const SceneObject& obj, int idx) {
    Vec3  pos    = getVec3Prop(obj, "position", {0, 1.5f, 0});
    float th     = getProp(obj, "trunk_height", 1.5f);
    float cr     = getProp(obj, "canopy_radius", 1.0f);
    Vec3  color  = getVec3Prop(obj, "color", {0.2f, 0.7f, 0.3f});

    Vec3 trunkSize = {0.3f, th, 0.3f};
    Vec3 trunkPos  = {pos.x, pos.y + th/2, pos.z};
    w.writeBox("tree_trunk_" + std::to_string(idx), trunkPos, trunkSize, {0.55f, 0.35f, 0.15f});

    Vec3 canopySize = {cr*2, cr*1.5f, cr*2};
    Vec3 canopyPos  = {pos.x, pos.y + th + cr*0.75f, pos.z};
    w.writeBox("tree_canopy_" + std::to_string(idx), canopyPos, canopySize, color);
}

static void genSphere(OBJWriter& w, const SceneObject& obj, int idx) {
    Vec3  pos    = getVec3Prop(obj, "position", {0,0,0});
    float radius = getProp(obj, "radius", 1.0f);
    Vec3  color  = getVec3Prop(obj, "color", {0.6f, 0.6f, 0.9f});
    Vec3 size = {radius*2, radius*2, radius*2};
    w.writeBox("sphere_approx_" + std::to_string(idx), pos, size, color);
}

static void genPlane(OBJWriter& w, const SceneObject& obj, int idx) {
    Vec3  pos   = getVec3Prop(obj, "position", {0,0,0});
    Vec3  size  = getVec3Prop(obj, "size", {4, 0.05f, 4});
    Vec3  color = getVec3Prop(obj, "color", {0.8f, 0.8f, 0.8f});
    w.writeBox("plane_" + std::to_string(idx), pos, size, color);
}

void generateOBJ(const SceneDescription& scene,
                 const std::string& objPath,
                 const std::string& mtlPath) {
    srand(42);
    OBJWriter w(objPath, mtlPath);

    int counts[6] = {};
    for (auto& obj : scene.objects) {
        int idx = counts[(int)obj.type]++;
        switch (obj.type) {
            case ObjectType::ISLAND: genIsland(w, obj, idx); break;
            case ObjectType::HOUSE:  genHouse (w, obj, idx); break;
            case ObjectType::TREE:   genTree  (w, obj, idx); break;
            case ObjectType::SPHERE: genSphere(w, obj, idx); break;
            case ObjectType::PLANE:  genPlane (w, obj, idx); break;
            case ObjectType::CAMERA: break;
        }
    }
}
