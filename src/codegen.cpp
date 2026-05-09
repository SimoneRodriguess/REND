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

static float noise(float x, float z) {
    int xi = (int)(x * 3.7f) ^ (int)(z * 6.1f);
    return ((xi * 1664525 + 1013904223) & 0x7fffffff) / (float)0x7fffffff * 0.18f;
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
        mtl << "Ka 0.1 0.1 0.1\nKs 0.0 0.0 0.0\n\n";
        return name;
    }

    void writeBox(const std::string& name, Vec3 pos, Vec3 size, Vec3 color) {
        float hw = size.x/2, hh = size.y/2, hd = size.z/2;
        float x = pos.x, y = pos.y, z = pos.z;
        std::string mat = newMat(color);
        obj << "o " << name << "\nusemtl " << mat << "\n";
        float verts[8][3] = {
            {x-hw,y-hh,z+hd},{x+hw,y-hh,z+hd},{x+hw,y+hh,z+hd},{x-hw,y+hh,z+hd},
            {x-hw,y-hh,z-hd},{x+hw,y-hh,z-hd},{x+hw,y+hh,z-hd},{x-hw,y+hh,z-hd},
        };
        for (auto& v : verts)
            obj << "v " << v[0] << " " << v[1] << " " << v[2] << "\n";
        int o = vertOffset;
        obj << "f " << o+3<<" "<<o+2<<" "<<o+1<<" "<<o+0<<"\n";
        obj << "f " << o+4<<" "<<o+5<<" "<<o+6<<" "<<o+7<<"\n";
        obj << "f " << o+0<<" "<<o+1<<" "<<o+5<<" "<<o+4<<"\n";
        obj << "f " << o+2<<" "<<o+3<<" "<<o+7<<" "<<o+6<<"\n";
        obj << "f " << o+1<<" "<<o+2<<" "<<o+6<<" "<<o+5<<"\n";
        obj << "f " << o+3<<" "<<o+0<<" "<<o+4<<" "<<o+7<<"\n\n";
        vertOffset += 8;
    }

    void writeIsland(const std::string& name, Vec3 size, float taper, Vec3 color) {
        float hw = size.x/2, hh = size.y/2, hd = size.z/2;
        float tw = hw * taper, td = hd * taper;
        std::string mat = newMat(color);
        obj << "o " << name << "\nusemtl " << mat << "\n";

        int gridN = 4;
        float stepX = size.x / gridN, stepZ = size.z / gridN;
        std::vector<std::vector<int>> topIdx(gridN+1, std::vector<int>(gridN+1));
        for (int iz = 0; iz <= gridN; iz++) {
            for (int ix = 0; ix <= gridN; ix++) {
                float fx = -hw + ix * stepX;
                float fz = -hd + iz * stepZ;
                float fy = hh + noise(fx, fz);
                obj << "v " << fx << " " << fy << " " << fz << "\n";
                topIdx[iz][ix] = vertOffset++;
            }
        }

        int b0 = vertOffset;
        obj << "v " << -tw << " " << -hh << " " <<  td << "\n";
        obj << "v " <<  tw << " " << -hh << " " <<  td << "\n";
        obj << "v " <<  tw << " " << -hh << " " << -td << "\n";
        obj << "v " << -tw << " " << -hh << " " << -td << "\n";
        vertOffset += 4;

        for (int iz = 0; iz < gridN; iz++)
            for (int ix = 0; ix < gridN; ix++)
                obj << "f " << topIdx[iz][ix] << " " << topIdx[iz][ix+1] << " "
                             << topIdx[iz+1][ix+1] << " " << topIdx[iz+1][ix] << "\n";

        obj << "f " << b0+3<<" "<<b0+2<<" "<<b0+1<<" "<<b0<<"\n";

        int tbl = topIdx[0][0], tbr = topIdx[0][gridN];
        int ttr = topIdx[gridN][gridN], ttl = topIdx[gridN][0];
        obj << "f " << tbl<<" "<<tbr<<" "<<b0+1<<" "<<b0+0<<"\n";
        obj << "f " << tbr<<" "<<ttr<<" "<<b0+2<<" "<<b0+1<<"\n";
        obj << "f " << ttr<<" "<<ttl<<" "<<b0+3<<" "<<b0+2<<"\n";
        obj << "f " << ttl<<" "<<tbl<<" "<<b0+0<<" "<<b0+3<<"\n\n";
    }

    void writeHouse(const std::string& name, Vec3 pos, Vec3 size,
                    float roofHeight, Vec3 wallColor, Vec3 roofColor) {
        // walls
        writeBox(name + "_walls", pos, size, wallColor);

        // pyramid roof — base sits on top of walls, peak in the center
        float hw = size.x/2, hd = size.z/2;
        float baseY = pos.y + size.y/2;
        float peakY = baseY + roofHeight;

        std::string mat = newMat(roofColor);
        obj << "o " << name << "_roof\nusemtl " << mat << "\n";

        // 4 base corners + 1 apex
        obj << "v " << pos.x-hw << " " << baseY << " " << pos.z+hd << "\n";
        obj << "v " << pos.x+hw << " " << baseY << " " << pos.z+hd << "\n";
        obj << "v " << pos.x+hw << " " << baseY << " " << pos.z-hd << "\n";
        obj << "v " << pos.x-hw << " " << baseY << " " << pos.z-hd << "\n";
        obj << "v " << pos.x    << " " << peakY << " " << pos.z    << "\n";

        int o = vertOffset;
        // 4 triangular faces
        obj << "f " << o+0<<" "<<o+1<<" "<<o+4<<"\n";
        obj << "f " << o+1<<" "<<o+2<<" "<<o+4<<"\n";
        obj << "f " << o+2<<" "<<o+3<<" "<<o+4<<"\n";
        obj << "f " << o+3<<" "<<o+0<<" "<<o+4<<"\n\n";

        vertOffset += 5;
    }
};

static void genIsland(OBJWriter& w, const SceneObject& obj, int idx) {
    Vec3  size  = getVec3Prop(obj, "size",  {8, 1.5f, 8});
    float taper = getProp(obj, "taper", 0.7f);
    Vec3  color = getVec3Prop(obj, "color", {0.35f, 0.68f, 0.25f});
    w.writeIsland("island_" + std::to_string(idx), size, taper, color);
}

static void genHouse(OBJWriter& w, const SceneObject& obj, int idx) {
    Vec3  pos       = getVec3Prop(obj, "position",   {0, 1.5f, 0});
    Vec3  size      = getVec3Prop(obj, "size",        {3, 2, 3});
    float roofH     = getProp(obj, "roof_height",     1.5f);
    Vec3  wallColor = getVec3Prop(obj, "color",       {0.9f, 0.9f, 0.85f});
    Vec3  roofColor = getVec3Prop(obj, "roof_color",  {0.72f, 0.35f, 0.2f});
    w.writeHouse("house_" + std::to_string(idx), pos, size, roofH, wallColor, roofColor);
}

static void genTree(OBJWriter& w, const SceneObject& obj, int idx) {
    Vec3  pos   = getVec3Prop(obj, "position",    {0, 1.5f, 0});
    float th    = getProp(obj, "trunk_height",     1.5f);
    float cr    = getProp(obj, "canopy_radius",    1.0f);
    Vec3  color = getVec3Prop(obj, "color",        {0.2f, 0.7f, 0.3f});
    Vec3 trunkPos  = {pos.x, pos.y + th/2,          pos.z};
    Vec3 canopyPos = {pos.x, pos.y + th + cr*0.75f, pos.z};
    w.writeBox("tree_trunk_"  + std::to_string(idx), trunkPos,  {0.3f, th, 0.3f},     {0.55f,0.35f,0.15f});
    w.writeBox("tree_canopy_" + std::to_string(idx), canopyPos, {cr*2, cr*1.5f, cr*2}, color);
}

static void genSphere(OBJWriter& w, const SceneObject& obj, int idx) {
    Vec3  pos    = getVec3Prop(obj, "position", {0,0,0});
    float radius = getProp(obj, "radius", 1.0f);
    Vec3  color  = getVec3Prop(obj, "color", {0.6f, 0.6f, 0.9f});
    w.writeBox("sphere_" + std::to_string(idx), pos, {radius*2,radius*2,radius*2}, color);
}

static void genPlane(OBJWriter& w, const SceneObject& obj, int idx) {
    Vec3 pos   = getVec3Prop(obj, "position", {0,0,0});
    Vec3 size  = getVec3Prop(obj, "size",     {4,0.05f,4});
    Vec3 color = getVec3Prop(obj, "color",    {0.8f,0.8f,0.8f});
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
