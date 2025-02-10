#include "hpp/Terrain.hpp"
#include "stb_image.h"
#include <vector>
#include <iostream>
#include "hpp/TextureLoader.hpp"

namespace Terrain {
    GLuint terrainVAO, terrainVBO, terrainIBO;
    GLint numStrips, numTrisPerStrip;
    GLuint grassColor, grassAO, grassRoughness, grassNormal, grassDisplacement;
    GLuint rockColor, rockAO, rockRoughness, rockNormal, rockDisplacement;
    GLuint snowColor, snowAO, snowRoughness, snowNormal, snowDisplacement;
    bool lightingEnabled = true;



    void initTerrain() {
        // Tworzenie mesh na podstawie heightmapy
        stbi_set_flip_vertically_on_load(true);
        GLint width, height, nrChannels;
        unsigned char* data = stbi_load("src/Textures/Terrain/Heightmaps/iceland_heightmap.png", &width, &height, &nrChannels, 0);
        if (data)
        {
            std::cout << "Loaded heightmap of size " << height << " x " << width << std::endl;
        }
        else
        {
            std::cout << "Failed to load texture" << std::endl;
        }


        std::vector<float> vertices;
        float yScale = 64.0f / 256.0f, yShift = 16.0f;
        int rez = 1;
        unsigned bytePerPixel = nrChannels;
        for (int i = 0; i < height; i++)
        {
            for (int j = 0; j < width; j++)
            {
                unsigned char* pixelOffset = data + (j + width * i) * bytePerPixel;
                unsigned char y = pixelOffset[0];


                float scale = 1.0f;
                vertices.push_back((-height / 2.0f + height * i / (float)height) * scale);   // vx
                vertices.push_back((int)y * yScale - yShift);   // vy
                vertices.push_back((-width / 2.0f + width * j / (float)width) * scale);  //vz
            }
        }
        std::cout << "Loaded " << vertices.size() / 3 << " vertices" << std::endl;
        stbi_image_free(data);

        std::vector<float> normals(vertices.size(), 0.0f); // Wektor normalnych

        // Iteracja po każdym wierzchołku i obliczanie normalnych
        for (int i = 1; i < height - 1; i++) {
            for (int j = 1; j < width - 1; j++) {
                int index = (j + width * i);
                glm::vec3 left(vertices[(index - 1) * 3], vertices[(index - 1) * 3 + 1], vertices[(index - 1) * 3 + 2]);
                glm::vec3 right(vertices[(index + 1) * 3], vertices[(index + 1) * 3 + 1], vertices[(index + 1) * 3 + 2]);
                glm::vec3 up(vertices[(index - width) * 3], vertices[(index - width) * 3 + 1], vertices[(index - width) * 3 + 2]);
                glm::vec3 down(vertices[(index + width) * 3], vertices[(index + width) * 3 + 1], vertices[(index + width) * 3 + 2]);

                glm::vec3 normal = glm::normalize(glm::cross(right - left, down - up));

                normals[index * 3] = normal.x;
                normals[index * 3 + 1] = normal.y;
                normals[index * 3 + 2] = normal.z;
            }
        }


        std::vector<unsigned> indices;
        for (unsigned i = 0; i < height - 1; i += rez)
        {
            for (unsigned j = 0; j < width; j += rez)
            {
                for (unsigned k = 0; k < 2; k++)
                {
                    indices.push_back(j + width * (i + k * rez));
                }
            }
        }


        std::cout << "Loaded " << indices.size() << " indices" << std::endl;

        std::vector<float> tangents(vertices.size(), 0.0f);
        std::vector<float> bitangents(vertices.size(), 0.0f);

        for (int i = 1; i < height - 1; i++) {
            for (int j = 1; j < width - 1; j++) {
                int index = (j + width * i);

                glm::vec3 pos1(vertices[(index) * 3], vertices[(index) * 3 + 1], vertices[(index) * 3 + 2]);
                glm::vec3 pos2(vertices[(index + 1) * 3], vertices[(index + 1) * 3 + 1], vertices[(index + 1) * 3 + 2]);
                glm::vec3 pos3(vertices[(index + width) * 3], vertices[(index + width) * 3 + 1], vertices[(index + width) * 3 + 2]);

                glm::vec2 uv1(j / (float)width, i / (float)height);
                glm::vec2 uv2((j + 1) / (float)width, i / (float)height);
                glm::vec2 uv3(j / (float)width, (i + 1) / (float)height);

                glm::vec3 edge1 = pos2 - pos1;
                glm::vec3 edge2 = pos3 - pos1;
                glm::vec2 deltaUV1 = uv2 - uv1;
                glm::vec2 deltaUV2 = uv3 - uv1;

                float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

                glm::vec3 tangent, bitangent;
                tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
                tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
                tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
                tangent = glm::normalize(tangent);

                bitangent.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
                bitangent.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
                bitangent.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
                bitangent = glm::normalize(bitangent);

                tangents[index * 3] = tangent.x;
                tangents[index * 3 + 1] = tangent.y;
                tangents[index * 3 + 2] = tangent.z;

                bitangents[index * 3] = bitangent.x;
                bitangents[index * 3 + 1] = bitangent.y;
                bitangents[index * 3 + 2] = bitangent.z;
            }
        }



        numStrips = (height - 1) / rez;
        numTrisPerStrip = (width / rez) * 2 - 2;
        std::cout << "Created lattice of " << numStrips << " strips with " << numTrisPerStrip << " triangles each" << std::endl;
        std::cout << "Created " << numStrips * numTrisPerStrip << " triangles total" << std::endl;

        // Konfigurowanie VAO VBO IBO
        glGenVertexArrays(1, &terrainVAO);
        glBindVertexArray(terrainVAO);

        glGenBuffers(1, &terrainVBO);
        glBindBuffer(GL_ARRAY_BUFFER, terrainVBO);
        glBufferData(GL_ARRAY_BUFFER,
            vertices.size() * sizeof(float) +
            normals.size() * sizeof(float) +
            tangents.size() * sizeof(float) +
            bitangents.size() * sizeof(float),
            nullptr, GL_STATIC_DRAW);

        glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(float), &vertices[0]);
        glBufferSubData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), normals.size() * sizeof(float), &normals[0]);
        glBufferSubData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float) + normals.size() * sizeof(float), tangents.size() * sizeof(float), &tangents[0]);
        glBufferSubData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float) + normals.size() * sizeof(float) + tangents.size() * sizeof(float), bitangents.size() * sizeof(float), &bitangents[0]);

        // Atrybuty wierzchołków
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        // Atrybuty normalnych
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)(vertices.size() * sizeof(float)));
        glEnableVertexAttribArray(1);

        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)(vertices.size() * sizeof(float) + normals.size() * sizeof(float)));
        glEnableVertexAttribArray(2);

        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)(vertices.size() * sizeof(float) + normals.size() * sizeof(float) + tangents.size() * sizeof(float)));
        glEnableVertexAttribArray(3);


        glGenBuffers(1, &terrainIBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, terrainIBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned), &indices[0], GL_STATIC_DRAW);

        // Ładowanie tekstur
        grassNormal = loadTexture("src/Textures/Terrain/Grass/Grass001_1K-PNG_NormalGL.png");

        rockNormal = loadTexture("src/Textures/Terrain/Rock/Rock020_1K-PNG_NormalGL.png");

        snowNormal = loadTexture("src/Textures/Terrain/Snow/Snow008A_1K-PNG_NormalGL.png");

        grassColor = loadTexture("src/Textures/Terrain/Grass/Grass001_1K-PNG_Color.png");

        rockColor = loadTexture("src/Textures/Terrain/Rock/Rock020_1K-PNG_Color.png");

        snowColor = loadTexture("src/Textures/Terrain/Snow/Snow008A_1K-PNG_Color.png");

    }

    void drawTerrain(GLuint program, const glm::mat4& viewMatrix, const glm::mat4& projMatrix, const glm::vec3& color, const glm::vec3& cameraPos, bool lightingEnabled) {

        //Rysowanie terenu
        glUseProgram(program);
        glUniformMatrix4fv(glGetUniformLocation(program, "projection"), 1, GL_FALSE, &projMatrix[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(program, "view"), 1, GL_FALSE, &viewMatrix[0][0]);
        glm::mat4 model = glm::mat4(1.0f);
        glUniformMatrix4fv(glGetUniformLocation(program, "model"), 1, GL_FALSE, &model[0][0]);


        glUniform1i(glGetUniformLocation(program, "lightingEnabled"), lightingEnabled);

        glUniform3fv(glGetUniformLocation(program, "viewPos"), 1, &cameraPos[0]);

        glm::vec3 lightColor(0.98f, 0.9f, 0.823f);
        glUniform3fv(glGetUniformLocation(program, "lightColor"), 1, &lightColor[0]);


        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, grassNormal);
        glUniform1i(glGetUniformLocation(program, "grassNormalMap"), 0);


        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, rockNormal);
        glUniform1i(glGetUniformLocation(program, "rockNormalMap"), 1);


        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, snowNormal);
        glUniform1i(glGetUniformLocation(program, "snowNormalMap"), 2);


        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, grassColor);
        glUniform1i(glGetUniformLocation(program, "grassColorMap"), 3);


        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, rockColor);
        glUniform1i(glGetUniformLocation(program, "rockColorMap"), 4);


        glActiveTexture(GL_TEXTURE5);
        glBindTexture(GL_TEXTURE_2D, snowColor);
        glUniform1i(glGetUniformLocation(program, "snowColorMap"), 5);



        glBindVertexArray(terrainVAO);
        //        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        for (unsigned strip = 0; strip < numStrips; strip++)
        {

            glDrawElements(GL_TRIANGLE_STRIP,
                numTrisPerStrip + 2,
                GL_UNSIGNED_INT,
                (void*)(sizeof(unsigned) * (numTrisPerStrip + 2) * strip));



        } glUseProgram(0);
    }


    void shutdownTerrain() {
        glDeleteVertexArrays(1, &terrainVAO);
        glDeleteBuffers(1, &terrainVBO);
        glDeleteBuffers(1, &terrainIBO);
    }
}
