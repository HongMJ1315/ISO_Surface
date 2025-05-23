#ifndef ISO_SURFACE_H
#define ISO_SURFACE_H

#include "glm/glm.hpp"
#include <iostream>
#include <vector>
#include <array>



class Iso_Surface{
    typedef unsigned char byte;
private:
struct VertData{
    int x, y, z;
};
struct Cube_MinMax{
    float min, max;
    int x, y, z;
};
    std::vector<std::vector<std::vector<float> > > data;
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;
    std::vector<Cube_MinMax> minmax;
    std::vector<float> distribute;
    float isovalue;
    int length, width, height;
    std::vector<glm::vec3> get_vert_tetr(VertData v0, VertData v1, VertData v2, VertData v3);
    std::vector<glm::vec3> get_vert_cube(VertData v0, VertData v1, VertData v2, VertData v3,
                                         VertData v4, VertData v5, VertData v6, VertData v7);
    void calculate_normals();
    void calculate_gradient();
public:
    Iso_Surface();
    Iso_Surface(std::vector<unsigned char> data, int length, int width, int height);
    void generate_tetr(float isovalue);
    void generate_cube(float isovalue);
    std::vector<glm::vec3> get_vertices();
    glm::vec3 compute_gradient_at(const glm::vec3 &pos);
    float sample_value(float x, float y, float z);
    std::vector<glm::vec3> get_normals();
    ~Iso_Surface();
    std::vector<float> get_distribute();
};


struct Surface{
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;
    glm::vec3 color;
};

#endif