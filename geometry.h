#ifndef GEOMETRY_H
#define GEOMETRY_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <cmath>

unsigned int cubeVAO, cubeVBO, cylVAO, cylVBO, cylVerts, sphVAO, sphVBO, sphVerts;

void setupGeom() {
    float cVerts[] = {
        // Positions         // Normals        // Tex Coords
        -0.5f,-0.5f,-0.5f,  0, 0,-1,  0, 0,
         0.5f,-0.5f,-0.5f,  0, 0,-1,  1, 0,
         0.5f, 0.5f,-0.5f,  0, 0,-1,  1, 1,
         0.5f, 0.5f,-0.5f,  0, 0,-1,  1, 1,
        -0.5f, 0.5f,-0.5f,  0, 0,-1,  0, 1,
        -0.5f,-0.5f,-0.5f,  0, 0,-1,  0, 0,
        
        -0.5f,-0.5f, 0.5f,  0, 0, 1,  0, 0,
         0.5f,-0.5f, 0.5f,  0, 0, 1,  1, 0,
         0.5f, 0.5f, 0.5f,  0, 0, 1,  1, 1,
         0.5f, 0.5f, 0.5f,  0, 0, 1,  1, 1,
        -0.5f, 0.5f, 0.5f,  0, 0, 1,  0, 1,
        -0.5f,-0.5f, 0.5f,  0, 0, 1,  0, 0,
        
        -0.5f, 0.5f, 0.5f, -1, 0, 0,  1, 0,
        -0.5f, 0.5f,-0.5f, -1, 0, 0,  1, 1,
        -0.5f,-0.5f,-0.5f, -1, 0, 0,  0, 1,
        -0.5f,-0.5f,-0.5f, -1, 0, 0,  0, 1,
        -0.5f,-0.5f, 0.5f, -1, 0, 0,  0, 0,
        -0.5f, 0.5f, 0.5f, -1, 0, 0,  1, 0,
        
         0.5f, 0.5f, 0.5f,  1, 0, 0,  1, 0,
         0.5f, 0.5f,-0.5f,  1, 0, 0,  1, 1,
         0.5f,-0.5f,-0.5f,  1, 0, 0,  0, 1,
         0.5f,-0.5f,-0.5f,  1, 0, 0,  0, 1,
         0.5f,-0.5f, 0.5f,  1, 0, 0,  0, 0,
         0.5f, 0.5f, 0.5f,  1, 0, 0,  1, 0,
        
        -0.5f,-0.5f,-0.5f,  0,-1, 0,  0, 1,
         0.5f,-0.5f,-0.5f,  0,-1, 0,  1, 1,
         0.5f,-0.5f, 0.5f,  0,-1, 0,  1, 0,
         0.5f,-0.5f, 0.5f,  0,-1, 0,  1, 0,
        -0.5f,-0.5f, 0.5f,  0,-1, 0,  0, 0,
        -0.5f,-0.5f,-0.5f,  0,-1, 0,  0, 1,
        
        -0.5f, 0.5f,-0.5f,  0, 1, 0,  0, 1,
         0.5f, 0.5f,-0.5f,  0, 1, 0,  1, 1,
         0.5f, 0.5f, 0.5f,  0, 1, 0,  1, 0,
         0.5f, 0.5f, 0.5f,  0, 1, 0,  1, 0,
        -0.5f, 0.5f, 0.5f,  0, 1, 0,  0, 0,
        -0.5f, 0.5f,-0.5f,  0, 1, 0,  0, 1
    };
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);
    glBindVertexArray(cubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cVerts), cVerts, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)0); glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)(3*sizeof(float))); glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)(6*sizeof(float))); glEnableVertexAttribArray(2);
    
    // Cylinder
    std::vector<float> cyl;
    int cylSegments = 36;
    for(int i=0; i<cylSegments; i++) {
        float rot1 = (float)i/cylSegments*2.0f*3.14159f;
        float rot2 = (float)(i+1)/cylSegments*2.0f*3.14159f;
        glm::vec3 n1((float)cos(rot1), 0, (float)sin(rot1));
        glm::vec3 n2((float)cos(rot2), 0, (float)sin(rot2));
        auto push = [&](glm::vec3 p, glm::vec3 n, float u, float v){
            cyl.push_back(p.x); cyl.push_back(p.y); cyl.push_back(p.z); 
            cyl.push_back(n.x); cyl.push_back(n.y); cyl.push_back(n.z); 
            cyl.push_back(u); cyl.push_back(v); 
        };
        push(glm::vec3(n1.x, 0.5f, n1.z), n1, 0, 1);
        push(glm::vec3(n1.x, -0.5f, n1.z), n1, 0, 0);
        push(glm::vec3(n2.x, 0.5f, n2.z), n2, 1, 1);
        
        push(glm::vec3(n2.x, 0.5f, n2.z), n2, 1, 1);
        push(glm::vec3(n1.x, -0.5f, n1.z), n1, 0, 0);
        push(glm::vec3(n2.x, -0.5f, n2.z), n2, 1, 0);     
    }
    cylVerts = (unsigned int)(cyl.size() / 8);
    glGenVertexArrays(1, &cylVAO);
    glGenBuffers(1, &cylVBO);
    glBindVertexArray(cylVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cylVBO);
    glBufferData(GL_ARRAY_BUFFER, cyl.size() * sizeof(float), cyl.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)0); glEnableVertexAttribArray(0); 
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)(3*sizeof(float))); glEnableVertexAttribArray(1); 
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)(6*sizeof(float))); glEnableVertexAttribArray(2);
    
    // Sphere
    std::vector<float> sph;
    int N = 20, M = 20;
    for(int i=0; i<=N; i++) {
        float fI = (float)i/N;
        float theta = fI * 3.14159f;
        for(int j=0; j<=M; j++) {
            float fJ = (float)j/M;
            float phi = fJ * 2.0f * 3.14159f;
            glm::vec3 p((float)cos(phi)*(float)sin(theta), (float)cos(theta), (float)sin(phi)*(float)sin(theta));
            sph.push_back(p.x); sph.push_back(p.y); sph.push_back(p.z); 
            sph.push_back(p.x); sph.push_back(p.y); sph.push_back(p.z); 
            sph.push_back(fJ); sph.push_back(fI);
        }
    }
    std::vector<unsigned int> idx;
    for(int i=0; i<N; i++) {
        for(int j=0; j<M; j++) { 
            idx.push_back(i*(M+1)+j); 
            idx.push_back((i+1)*(M+1)+j); 
            idx.push_back((i+1)*(M+1)+j+1); 
            
            idx.push_back(i*(M+1)+j); 
            idx.push_back((i+1)*(M+1)+j+1); 
            idx.push_back(i*(M+1)+j+1); 
        }
    }
    sphVerts = (unsigned int)idx.size();
    unsigned int ebo;
    glGenVertexArrays(1, &sphVAO);
    glGenBuffers(1, &sphVBO);
    glGenBuffers(1, &ebo);
    
    glBindVertexArray(sphVAO);
    glBindBuffer(GL_ARRAY_BUFFER, sphVBO);
    glBufferData(GL_ARRAY_BUFFER, sph.size() * sizeof(float), sph.data(), GL_STATIC_DRAW);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, idx.size() * sizeof(unsigned int), idx.data(), GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)0); glEnableVertexAttribArray(0); 
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)(3*sizeof(float))); glEnableVertexAttribArray(1); 
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)(6*sizeof(float))); glEnableVertexAttribArray(2);
}

void drawMesh(unsigned int vao, unsigned int count, unsigned int shaderID, glm::mat4 mm, glm::vec3 col, float spec = 0.5f, unsigned int tex = 0, float alpha = 1.0f, bool isIndexed = false, glm::vec3 emissive = glm::vec3(0.0f)) {
    glUniformMatrix4fv(glGetUniformLocation(shaderID, "model"), 1, GL_FALSE, glm::value_ptr(mm));
    
    if(tex != 0) { 
        glActiveTexture(GL_TEXTURE0); 
        glBindTexture(GL_TEXTURE_2D, tex); 
        glUniform1i(glGetUniformLocation(shaderID, "material.useTexture"), 1); 
        glUniform1i(glGetUniformLocation(shaderID, "material.texture_diffuse1"), 0); 
    } else { 
        glUniform3f(glGetUniformLocation(shaderID, "material.diffuse"), col.x, col.y, col.z); 
        glUniform1i(glGetUniformLocation(shaderID, "material.useTexture"), 0); 
    }
    
    glUniform3f(glGetUniformLocation(shaderID, "material.specular"), spec, spec, spec); 
    glUniform1f(glGetUniformLocation(shaderID, "material.shininess"), 32.0f); 
    glUniform1f(glGetUniformLocation(shaderID, "material.alpha"), alpha);
    glUniform3f(glGetUniformLocation(shaderID, "material.emissive"), emissive.x, emissive.y, emissive.z);
    
    glBindVertexArray(vao);
    if(isIndexed) {
        glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, 0);
    } else {
        glDrawArrays(GL_TRIANGLES, 0, count);
    }
    glBindVertexArray(0);
}

#endif
