#include"iso_surface.h"
#include "reader.h"
#include "MarchingCubesTables.hpp"
#include "MarchingTetrahedraTables.hpp"
#include <algorithm>

Iso_Surface::Iso_Surface(){
    this->length = 0;
    this->width = 0;
    this->height = 0;
    this->isovalue = 0.0f;
}

Iso_Surface::Iso_Surface(std::vector<unsigned char> data, int length, int width, int height){
    distribute.resize(256, 0);
    
    float max_val = -1.0f;

    for(int i = 0; i < length; i++){
        std::vector<std::vector<float> > temp;
        for(int j = 0; j < width; j++){
            std::vector<float> temp2;
            for(int k = 0; k < height; k++){
                temp2.push_back((float)int(data[i * width * height + j * height + k]));
                if(int(data[i * width * height + j * height + k]) > 0)
                    distribute[int(data[i * width * height + j * height + k])]+=1;
            }
            temp.push_back(temp2);
        }
        this->data.push_back(temp);
    }
    for(auto i: distribute){
        max_val = std::max(max_val, i);
    }
    std::cout << max_val << std::endl;
    for(auto &i: distribute){
        std::cout << i << std::endl;
        i /= max_val;
    }

    for(int i = 0; i < length - 1; i++){
        for(int j = 0; j < width - 1; j++){
            for(int k = 0; k < height - 1; k++){
                VertData v_data[] = {{i, j, k + 1},
                {i + 1, j, k + 1},
                {i + 1, j + 1, k + 1},
                {i, j + 1, k + 1},
                {i, j, k},
                {i + 1, j, k},
                {i + 1, j + 1, k},
                {i, j + 1, k}};
                float min = 256.0f, max = -1.0f;
                VertData min_loc = {0, 0, 0};
                VertData max_loc = {0, 0, 0};
                for(int l = 0; l < 8; l++){
                    if(this->data[v_data[l].x][v_data[l].y][v_data[l].z] < min){
                        min = this->data[v_data[l].x][v_data[l].y][v_data[l].z];
                        min_loc = v_data[l];
                    }
                    if(this->data[v_data[l].x][v_data[l].y][v_data[l].z] > max){
                        max = this->data[v_data[l].x][v_data[l].y][v_data[l].z];
                        max_loc = v_data[l];
                    }
                }
                minmax.push_back({min, max, min_loc.x, min_loc.y, min_loc.z});
            }
        }
    }

    std::sort(minmax.begin(), minmax.end(), [](Cube_MinMax a, Cube_MinMax b){
        if(a.min != b.min)
            return a.min < b.min;
        else
            return a.max < b.max;
    });
    
    this->length = length;
    this->width = width;
    this->height = height;
    this->isovalue = 0.0f;
}

std::vector<glm::vec3> Iso_Surface::get_vert_tetr(VertData v0, VertData v1, VertData v2, VertData v3){
    std::vector<glm::vec3> verts;
    short index = 0;

    
    if(data[v0.x][v0.y][v0.z] > isovalue){
        index |= 1;
    }
    
    if(data[v1.x][v1.y][v1.z] > isovalue){
        index |= 2;
    }

    if(data[v2.x][v2.y][v2.z] > isovalue){
        index |= 4;
    }

    if(data[v3.x][v3.y][v3.z] > isovalue){
        index |= 8;
    }

    std::array<short, 7> edges = tetr::kTetrahedraTriangles[index];
    for(int i = 0; i < 7; i++){
        short e = edges[i];
        
        if(e == -1) continue;
        /*
        else if(e == 0){
            verts.push_back(glm::vec3((v0.x + v1.x) / 2.f,
                                      (v0.y + v1.y) / 2.f,
                                      (v0.z + v1.z) / 2.f));
        }
        else if(e == 1){
            verts.push_back(glm::vec3((v1.x + v2.x) / 2.f,
                                      (v1.y + v2.y) / 2.f,
                                      (v1.z + v2.z) / 2.f));
        }
        else if(e == 2){
            verts.push_back(glm::vec3((v2.x + v0.x) / 2.f,
                                      (v2.y + v0.y) / 2.f,
                                      (v2.z + v0.z) / 2.f));
        }
        else if(e == 3){
            verts.push_back(glm::vec3((v3.x + v0.x) / 2.f,
                                      (v3.y + v0.y) / 2.f,
                                      (v3.z + v0.z) / 2.f));
        }
        else if(e == 4){
            verts.push_back(glm::vec3((v3.x + v1.x) / 2.f,
                                      (v3.y + v1.y) / 2.f,
                                      (v3.z + v1.z) / 2.f));
        }
        else if(e == 5){
            verts.push_back(glm::vec3((v2.x + v3.x) / 2.f,
                                      (v2.y + v3.y) / 2.f,
                                      (v2.z + v3.z) / 2.f));
        }

        // */
        // /*
        else if(e == 0){
            float val0 = data[v0.x][v0.y][v0.z];
            float val1 = data[v1.x][v1.y][v1.z];
            float alpha = (isovalue - val0) / (val1 - val0);
            glm::vec3 v = glm::vec3(v0.x + alpha * (v1.x - v0.x),
                                    v0.y + alpha * (v1.y - v0.y),
                                    v0.z + alpha * (v1.z - v0.z));
            verts.push_back(v);
        }
        else if(e == 1){
            float val0 = data[v1.x][v1.y][v1.z];
            float val1 = data[v2.x][v2.y][v2.z];
            float alpha = (isovalue - val0) / (val1 - val0);
            glm::vec3 v = glm::vec3(v1.x + alpha * (v2.x - v1.x),
                                    v1.y + alpha * (v2.y - v1.y),
                                    v1.z + alpha * (v2.z - v1.z));
            verts.push_back(v);
        }
        else if(e == 2){
            float val0 = data[v0.x][v0.y][v0.z];
            float val1 = data[v2.x][v2.y][v2.z];
            float alpha = (isovalue - val0) / (val1 - val0);
            glm::vec3 v = glm::vec3(v0.x + alpha * (v2.x - v0.x),
                                    v0.y + alpha * (v2.y - v0.y),
                                    v0.z + alpha * (v2.z - v0.z));
            verts.push_back(v);
        }
        else if(e == 3){
            float val0 = data[v0.x][v0.y][v0.z];
            float val1 = data[v3.x][v3.y][v3.z];
            float alpha = (isovalue - val0) / (val1 - val0);
            glm::vec3 v = glm::vec3(v0.x + alpha * (v3.x - v0.x),
                                    v0.y + alpha * (v3.y - v0.y),
                                    v0.z + alpha * (v3.z - v0.z));
            verts.push_back(v);
        }
        else if(e == 4){
            float val0 = data[v1.x][v1.y][v1.z];
            float val1 = data[v3.x][v3.y][v3.z];
            float alpha = (isovalue - val0) / (val1 - val0);
            glm::vec3 v = glm::vec3(v1.x + alpha * (v3.x - v1.x),
                                    v1.y + alpha * (v3.y - v1.y),
                                    v1.z + alpha * (v3.z - v1.z));
            verts.push_back(v);
        }
        else if(e == 5){
            float val0 = data[v2.x][v2.y][v2.z];
            float val1 = data[v3.x][v3.y][v3.z];
            float alpha = (isovalue - val0) / (val1 - val0);
            glm::vec3 v = glm::vec3(v2.x + alpha * (v3.x - v2.x),
                                    v2.y + alpha * (v3.y - v2.y),
                                    v2.z + alpha * (v3.z - v2.z));
            verts.push_back(v);
        }
        // */
    }
    return verts;
} 

std::vector<glm::vec3> Iso_Surface::get_vert_cube(VertData v0, VertData v1, VertData v2, VertData v3,
                                         VertData v4, VertData v5, VertData v6, VertData v7){
    std::vector<glm::vec3> verts;
    short index = 0;

    if(data[v0.x][v0.y][v0.z] > isovalue){
        index |= 1;
    }
    
    if(data[v1.x][v1.y][v1.z] > isovalue){
        index |= 2;
    }

    if(data[v2.x][v2.y][v2.z] > isovalue){
        index |= 4;
    }

    if(data[v3.x][v3.y][v3.z] > isovalue){
        index |= 8;
    }

    if(data[v4.x][v4.y][v4.z] > isovalue){
        index |= 16;
    }

    if(data[v5.x][v5.y][v5.z] > isovalue){
        index |= 32;
    }

    if(data[v6.x][v6.y][v6.z] > isovalue){
        index |= 64;
    }

    if(data[v7.x][v7.y][v7.z] > isovalue){
        index |= 128;
    }

    std::array<short, 16> edges = cube::kCubeTriangles[index];
    for(int i = 0; i < 16; i++){
        short e = edges[i];
        if(e == -1) continue;
        // e = 0 v0->v1
        else if(e == 0){
            float val0 = data[v0.x][v0.y][v0.z];
            float val1 = data[v1.x][v1.y][v1.z];
            float alpha = (isovalue - val0) / (val1 - val0);
            glm::vec3 v = glm::vec3(v0.x + alpha * (v1.x - v0.x),
                                    v0.y + alpha * (v1.y - v0.y),
                                    v0.z + alpha * (v1.z - v0.z));
            verts.push_back(v);
        }
        // e = 1 v1->v2
        else if(e == 1){
            float val0 = data[v1.x][v1.y][v1.z];
            float val1 = data[v2.x][v2.y][v2.z];
            float alpha = (isovalue - val0) / (val1 - val0);
            glm::vec3 v = glm::vec3(v1.x + alpha * (v2.x - v1.x),
                                    v1.y + alpha * (v2.y - v1.y),
                                    v1.z + alpha * (v2.z - v1.z));
            verts.push_back(v);
        }
        // e = 2 v2->v3
        else if(e == 2){
            float val0 = data[v2.x][v2.y][v2.z];
            float val1 = data[v3.x][v3.y][v3.z];
            float alpha = (isovalue - val0) / (val1 - val0);
            glm::vec3 v = glm::vec3(v2.x + alpha * (v3.x - v2.x),
                                    v2.y + alpha * (v3.y - v2.y),
                                    v2.z + alpha * (v3.z - v2.z));
            verts.push_back(v);
        }
        // e = 3 v3->v0
        else if(e == 3){
            float val0 = data[v3.x][v3.y][v3.z];
            float val1 = data[v0.x][v0.y][v0.z];
            float alpha = (isovalue - val0) / (val1 - val0);
            glm::vec3 v = glm::vec3(v3.x + alpha * (v0.x - v3.x),
                                    v3.y + alpha * (v0.y - v3.y),
                                    v3.z + alpha * (v0.z - v3.z));
            verts.push_back(v);
        }
        // e = 4 v4->v5
        else if(e == 4){
            float val0 = data[v4.x][v4.y][v4.z];
            float val1 = data[v5.x][v5.y][v5.z];
            float alpha = (isovalue - val0) / (val1 - val0);
            glm::vec3 v = glm::vec3(v4.x + alpha * (v5.x - v4.x),
                                    v4.y + alpha * (v5.y - v4.y),
                                    v4.z + alpha * (v5.z - v4.z));
            verts.push_back(v);
        }
        // e = 5 v5->v6
        else if(e == 5){
            float val0 = data[v5.x][v5.y][v5.z];
            float val1 = data[v6.x][v6.y][v6.z];
            float alpha = (isovalue - val0) / (val1 - val0);
            glm::vec3 v = glm::vec3(v5.x + alpha * (v6.x - v5.x),
                                    v5.y + alpha * (v6.y - v5.y),
                                    v5.z + alpha * (v6.z - v5.z));
            verts.push_back(v);
        }
        // e = 6 v6->v7
        else if(e == 6){
            float val0 = data[v6.x][v6.y][v6.z];
            float val1 = data[v7.x][v7.y][v7.z];
            float alpha = (isovalue - val0) / (val1 - val0);
            glm::vec3 v = glm::vec3(v6.x + alpha * (v7.x - v6.x),
                                    v6.y + alpha * (v7.y - v6.y),
                                    v6.z + alpha * (v7.z - v6.z));
            verts.push_back(v);
        }
        // e = 7 v7->v4
        else if(e == 7){
            float val0 = data[v7.x][v7.y][v7.z];
            float val1 = data[v4.x][v4.y][v4.z];
            float alpha = (isovalue - val0) / (val1 - val0);
            glm::vec3 v = glm::vec3(v7.x + alpha * (v4.x - v7.x),
                                    v7.y + alpha * (v4.y - v7.y),
                                    v7.z + alpha * (v4.z - v7.z));
            verts.push_back(v);
        }
        // e = 8 v0->v4
        else if(e == 8){
            float val0 = data[v0.x][v0.y][v0.z];
            float val1 = data[v4.x][v4.y][v4.z];
            float alpha = (isovalue - val0) / (val1 - val0);
            glm::vec3 v = glm::vec3(v0.x + alpha * (v4.x - v0.x),
                                    v0.y + alpha * (v4.y - v0.y),
                                    v0.z + alpha * (v4.z - v0.z));
            verts.push_back(v);
        }
        // e = 9 v1->v5
        else if(e == 9){
            float val0 = data[v1.x][v1.y][v1.z];
            float val1 = data[v5.x][v5.y][v5.z];
            float alpha = (isovalue - val0) / (val1 - val0);
            glm::vec3 v = glm::vec3(v1.x + alpha * (v5.x - v1.x),
                                    v1.y + alpha * (v5.y - v1.y),
                                    v1.z + alpha * (v5.z - v1.z));
            verts.push_back(v);
        }
        // e = 10 v2->v6
        else if(e == 10){
            float val0 = data[v2.x][v2.y][v2.z];
            float val1 = data[v6.x][v6.y][v6.z];
            float alpha = (isovalue - val0) / (val1 - val0);
            glm::vec3 v = glm::vec3(v2.x + alpha * (v6.x - v2.x),
                                    v2.y + alpha * (v6.y - v2.y),
                                    v2.z + alpha * (v6.z - v2.z));
            verts.push_back(v);
        }
        // e = 11 v3->v7
        else if(e == 11){
            float val0 = data[v3.x][v3.y][v3.z];
            float val1 = data[v7.x][v7.y][v7.z];
            float alpha = (isovalue - val0) / (val1 - val0);
            glm::vec3 v = glm::vec3(v3.x + alpha * (v7.x - v3.x),
                                    v3.y + alpha * (v7.y - v3.y),
                                    v3.z + alpha * (v7.z - v3.z));
            verts.push_back(v);
        }
    }
    return verts;
}

void Iso_Surface::generate_tetr(float isovalue){
    this->isovalue = isovalue;
    for(int i = 0; i < length - 1; i++){
        for(int j = 0; j < width - 1; j++){
            for(int k = 0; k < height - 1; k++){
                VertData v0_data = {i, j, k + 1};
                VertData v1_data = {i + 1, j, k + 1};
                VertData v2_data = {i + 1, j + 1, k + 1};
                VertData v3_data = {i, j + 1, k + 1};
                VertData v4_data = {i, j, k};
                VertData v5_data = {i + 1, j, k};
                VertData v6_data = {i + 1, j + 1, k};
                VertData v7_data = {i, j + 1, k};


                std::vector<glm::vec3> verts0 = get_vert_tetr(v0_data, v5_data, v1_data, v2_data);
                std::vector<glm::vec3> verts1 = get_vert_tetr(v0_data, v5_data, v7_data, v4_data);
                std::vector<glm::vec3> verts2 = get_vert_tetr(v0_data, v3_data, v7_data, v2_data);
                std::vector<glm::vec3> verts3 = get_vert_tetr(v6_data, v5_data, v7_data, v2_data);
                std::vector<glm::vec3> verts4 = get_vert_tetr(v0_data, v5_data, v7_data, v2_data);
                
                for(int i = 0; i < verts0.size(); i++){
                    vertices.push_back(verts0[i]);
                }
                for(int i = 0; i < verts1.size(); i++){
                    vertices.push_back(verts1[i]);
                }
                for(int i = 0; i < verts2.size(); i++){
                    vertices.push_back(verts2[i]);
                }
                for(int i = 0; i < verts3.size(); i++){
                    vertices.push_back(verts3[i]);
                }
                for(int i = 0; i < verts4.size(); i++){
                    vertices.push_back(verts4[i]);
                }
                
            }
        }
    }   
    
    calculate_normals();
}

void Iso_Surface::generate_cube(float iso_value){
    std::cout << "generate_cube" << std::endl;
    vertices.clear();
    normals.clear();
    this->isovalue = iso_value;
    // /*
    for(int i = 0; i < length - 1; i++){
        for(int j = 0; j < width - 1; j++){
            for(int k = 0; k < height - 1; k++){
                VertData v0_data = {i, j, k + 1};
                VertData v1_data = {i + 1, j, k + 1};
                VertData v2_data = {i + 1, j + 1, k + 1};
                VertData v3_data = {i, j + 1, k + 1};
                VertData v4_data = {i, j, k};
                VertData v5_data = {i + 1, j, k};
                VertData v6_data = {i + 1, j + 1, k};
                VertData v7_data = {i, j + 1, k};

                std::vector<glm::vec3> verts = get_vert_cube(v0_data, v1_data, v2_data, v3_data,
                                                             v4_data, v5_data, v6_data, v7_data);
                for(int i = 0; i < verts.size(); i++){
                    vertices.push_back(verts[i]);
                }
            }
        }
    }
    // */
    calculate_gradient();
    std::cout << "generate_cube end" << std::endl;
}

float Iso_Surface::sample_value(float x, float y, float z) {
    int x0 = static_cast<int>(std::floor(x));
    int y0 = static_cast<int>(std::floor(y));
    int z0 = static_cast<int>(std::floor(z));
    int x1 = x0 + 1;
    int y1 = y0 + 1;
    int z1 = z0 + 1;

    x0 = std::clamp(x0, 0, length - 1);
    x1 = std::clamp(x1, 0, length - 1);
    y0 = std::clamp(y0, 0, width - 1);
    y1 = std::clamp(y1, 0, width - 1);
    z0 = std::clamp(z0, 0, height - 1);
    z1 = std::clamp(z1, 0, height - 1);

    float xd = x - x0;
    float yd = y - y0;
    float zd = z - z0;

    float c00 = data[x0][y0][z0] * (1 - xd) + data[x1][y0][z0] * xd;
    float c01 = data[x0][y0][z1] * (1 - xd) + data[x1][y0][z1] * xd;
    float c10 = data[x0][y1][z0] * (1 - xd) + data[x1][y1][z0] * xd;
    float c11 = data[x0][y1][z1] * (1 - xd) + data[x1][y1][z1] * xd;
    
    float c0 = c00 * (1 - yd) + c10 * yd;
    float c1 = c01 * (1 - yd) + c11 * yd;
    
    return c0 * (1 - zd) + c1 * zd;
}

glm::vec3 Iso_Surface::compute_gradient_at(const glm::vec3 &pos) {
    float delta = 0.5f;
    float gx = sample_value(pos.x + delta, pos.y, pos.z) - sample_value(pos.x - delta, pos.y, pos.z);
    float gy = sample_value(pos.x, pos.y + delta, pos.z) - sample_value(pos.x, pos.y - delta, pos.z);
    float gz = sample_value(pos.x, pos.y, pos.z + delta) - sample_value(pos.x, pos.y, pos.z - delta);
    
    glm::vec3 grad(gx, gy, gz);
    float len = glm::length(grad);
    if (len == 0.0f)
        return glm::vec3(0.0f, 0.0f, 0.0f);
    return grad / len;
}

void Iso_Surface::calculate_gradient() {
    normals.clear();
    for (size_t i = 0; i < vertices.size(); ++i) {
         glm::vec3 pos = vertices[i];
         glm::vec3 normal = compute_gradient_at(pos);
         normals.push_back(normal);
    }
}

void Iso_Surface::calculate_normals() {
    normals.clear();
    for (size_t i = 0; i < vertices.size(); i += 3) {
        glm::vec3 p0 = vertices[i];
        glm::vec3 p1 = vertices[i + 1];
        glm::vec3 p2 = vertices[i + 2];
        glm::vec3 edge1 = p1 - p0;
        glm::vec3 edge2 = p2 - p0;
        glm::vec3 normal = glm::normalize(glm::cross(edge1, edge2));
        normals.push_back(normal);
        normals.push_back(normal);
        normals.push_back(normal);
    }
}


Iso_Surface::~Iso_Surface(){
    vertices.clear();
    normals.clear();
    data.clear();
}

std::vector<glm::vec3> Iso_Surface::get_vertices(){
    return vertices;
}

std::vector<glm::vec3> Iso_Surface::get_normals(){
    return normals;
}

std::vector<float> Iso_Surface::get_distribute(){
    return distribute;
}