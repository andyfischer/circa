// Copyright 2009 Andrew Fischer

#ifndef CUTTLEFISH_MESH_INCLUDED
#define CUTTLEFISH_MESH_INCLUDED

struct Vertex2f
{
    float x;
    float y;
};

struct Vertex3f
{
    float x;
    float y;
    float z;
};

struct Face
{
    int vertex_index[4];
    int uv_index[4];

    int numVertices() const {
        return vertex_index[3] == -1 ? 3 : 4;
    }
};

struct Mesh
{
    std::vector<Vertex3f> vertices;
    std::vector<Vertex2f> uvs;
    std::vector<Face> faces;

    void clear() {
        vertices.clear();
        uvs.clear();
        faces.clear();
    }
};

namespace mesh {

void load_obj_file(std::string const& filename, Mesh& mesh);
void draw_mesh_immediate(Mesh& mesh);
void setup(circa::Branch& branch);

}

#endif
