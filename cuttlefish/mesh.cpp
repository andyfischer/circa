// Copyright 2009 Andrew Fischer

#include "SDL_opengl.h"

#include "circa.h"

#include "mesh.h"

using namespace circa;
using namespace circa::tokenizer;

float consume_float(TokenStream& tokens)
{
    return atof(tokens.consume(FLOAT).c_str());
}

int consume_int(TokenStream& tokens)
{
    return strtoul(tokens.consume(INTEGER).c_str(), NULL, 0);
}

void parse_vertex(Mesh& mesh, TokenStream& tokens)
{
    tokens.consume();
    tokens.consume(WHITESPACE);

    Vertex3f v;
    v.x = consume_float(tokens);
    tokens.consume(WHITESPACE);
    v.y = consume_float(tokens);
    tokens.consume(WHITESPACE);
    v.z = consume_float(tokens);

    mesh.vertices.push_back(v);
}

void parse_uv(Mesh& mesh, TokenStream& tokens)
{
    tokens.consume();
    tokens.consume(WHITESPACE);

    Vertex2f n;
    n.x = consume_float(tokens);
    tokens.consume(WHITESPACE);
    n.y = consume_float(tokens);

    mesh.uvs.push_back(n);
}

void parse_face(Mesh& mesh, TokenStream& tokens)
{
    tokens.consume();
    tokens.consume(WHITESPACE);

    Face face;

    for (int i=0; i < 4; i++) {
        face.vertex_index[i] = consume_int(tokens) - 1;

        if (tokens.nextIs(SLASH)) {
            tokens.consume(SLASH);
            face.uv_index[i] = consume_int(tokens) - 1;
        } else {
            face.uv_index[i] = -1;
        }

        if (tokens.nextIs(WHITESPACE))
            tokens.consume(WHITESPACE);

        if (tokens.nextIs(NEWLINE)) {
            for (int j=i+1; j < 4; j++) {
                face.vertex_index[j] = -1;
                face.uv_index[i] = -1;
            }
            break;
        }
    }

    mesh.faces.push_back(face);
}

void load_obj_file(std::string const& filename, Mesh& mesh)
{
    std::string fileContents = read_text_file(filename);

    TokenStream tokens(fileContents);

    while (!tokens.finished()) {
        std::string command = tokens.next().text;

        if (command == "v")
            parse_vertex(mesh, tokens);
        else if (command == "vt")
            parse_uv(mesh, tokens);
        else if (command == "f")
            parse_face(mesh, tokens);
        else
            // ignore any unrecognized lines
            while (!tokens.nextIs(NEWLINE) && !tokens.finished())
                tokens.consume();

        if (tokens.nextIs(NEWLINE))
            tokens.consume();
    }
}

void draw_mesh_immediate(Mesh& mesh)
{
    // for now we assume that all meshes are full of quads
    glBegin(GL_QUADS);
    for (std::vector<Face>::const_iterator it = mesh.faces.begin(); it != mesh.faces.end(); ++it)
    {
        Face const& face = *it;
        //glColor3f(1,1,1);
        for (int i=0; i < 4; i++) {
            assert(face.vertex_index[i] >= 0);
            Vertex3f const& vert = mesh.vertices[face.vertex_index[i]];
            Vertex2f const& tex = mesh.uvs[face.uv_index[i]];
            if (face.uv_index[i] != -1)
                glTexCoord2f(tex.x, tex.y);
            glVertex3f(vert.x, vert.y, vert.z);
        }
    }
    glEnd();
}
