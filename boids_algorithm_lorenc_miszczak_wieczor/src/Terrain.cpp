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
    bool lightingEnabled = true; // Domy?nie ?iat? w?czone



    void initTerrain() {
        // Wczytaj heightmap?
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


        // set up vertex data (and buffer(s)) and configure vertex attributes
        // ------------------------------------------------------------------
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

                // vertex
                float scale = 1.0f; // Skalowanie terenu (np. 50% oryginalnego rozmiaru)
                vertices.push_back((-height / 2.0f + height * i / (float)height) * scale);   // vx
                vertices.push_back((int)y * yScale - yShift);   // vy
                vertices.push_back((-width / 2.0f + width * j / (float)width) * scale);
            }
        }
        std::cout << "Loaded " << vertices.size() / 3 << " vertices" << std::endl;
        stbi_image_free(data);

        std::vector<float> normals(vertices.size(), 0.0f); // Wektor normalnych, na pocz?ek zerowy

        // Iteracja po ka?ym wierzcho?u i obliczanie normalnych
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

        numStrips = (height - 1) / rez;
        numTrisPerStrip = (width / rez) * 2 - 2;
        std::cout << "Created lattice of " << numStrips << " strips with " << numTrisPerStrip << " triangles each" << std::endl;
        std::cout << "Created " << numStrips * numTrisPerStrip << " triangles total" << std::endl;

        // first, configure the cube's VAO (and terrainVBO + terrainIBO)
        glGenVertexArrays(1, &terrainVAO);
        glBindVertexArray(terrainVAO);

        glGenBuffers(1, &terrainVBO);
        glBindBuffer(GL_ARRAY_BUFFER, terrainVBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float) + normals.size() * sizeof(float), nullptr, GL_STATIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(float), &vertices[0]);
        glBufferSubData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), normals.size() * sizeof(float), &normals[0]);

        // Atrybuty wierzcho??
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        // Atrybuty normalnych
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)(vertices.size() * sizeof(float)));
        glEnableVertexAttribArray(1);


        glGenBuffers(1, &terrainIBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, terrainIBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned), &indices[0], GL_STATIC_DRAW);

        grassColor = loadTexture("src/Textures/Terrain/Grass/Grass001_1K-PNG_Color.png");
        grassAO = loadTexture("src/Textures/Terrain/Grass/Grass001_1K-PNG_AmbientOcclusion.png");
        grassRoughness = loadTexture("src/Textures/Terrain/Grass/Grass001_1K-PNG_Roughness.png");
        grassNormal = loadTexture("src/Textures/Terrain/Grass/Grass001_1K-PNG_NormalGL.png");
        grassDisplacement = loadTexture("src/Textures/Terrain/Grass/Grass001_1K-PNG_Displacement.png");

        rockColor = loadTexture("src/Textures/Terrain/Rock/Rock020_1K-PNG_Color.png");
        rockAO = loadTexture("src/Textures/Terrain/Rock/Rock020_1K-PNG_AmbientOcclusion.png");
        rockRoughness = loadTexture("src/Textures/Terrain/Rock/Rock020_1K-PNG_Roughness.png");
        rockNormal = loadTexture("src/Textures/Terrain/Rock/Rock020_1K-PNG_NormalGL.png");
        rockDisplacement = loadTexture("src/Textures/Terrain/Rock/Rock020_1K-PNG_Displacement.png");

        snowColor = loadTexture("src/Textures/Terrain/Snow/Snow008A_1K-PNG_Color.png");
        snowAO = loadTexture("src/Textures/Terrain/Snow/Snow008A_1K-PNG_AmbientOcclusion.png");
        snowRoughness = loadTexture("src/Textures/Terrain/Snow/Snow008A_1K-PNG_Roughness.png");
        snowNormal = loadTexture("src/Textures/Terrain/Snow/Snow008A_1K-PNG_NormalGL.png");
        snowDisplacement = loadTexture("src/Textures/Terrain/Snow/Snow008A_1K-PNG_Displacement.png");
    }

    void drawTerrain(GLuint program, const glm::mat4& viewMatrix, const glm::mat4& projMatrix, const glm::vec3& color, const glm::vec3& cameraPos, bool lightingEnabled) {

        glUseProgram(program);
        glUniformMatrix4fv(glGetUniformLocation(program, "projection"), 1, GL_FALSE, &projMatrix[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(program, "view"), 1, GL_FALSE, &viewMatrix[0][0]);
        glm::mat4 model = glm::mat4(1.0f);
        glUniformMatrix4fv(glGetUniformLocation(program, "model"), 1, GL_FALSE, &model[0][0]);


        glUniform1i(glGetUniformLocation(program, "lightingEnabled"), lightingEnabled);
        glm::vec3 lightPos = glm::normalize(glm::vec3(-9.0f, 200.0f, 10.0f));
        glUniform3fv(glGetUniformLocation(program, "lightPos"), 1, &lightPos[0]);



        // Pozycja kamery
        glUniform3fv(glGetUniformLocation(program, "viewPos"), 1, &cameraPos[0]);

        // Kolor Œwiat³a
        glm::vec3 lightColor(0.98f, 0.9f, 0.823f);
        glUniform3fv(glGetUniformLocation(program, "lightColor"), 1, &lightColor[0]);


        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, grassColor);
        glUniform1i(glGetUniformLocation(program, "grassColor"), 0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, grassAO);
        glUniform1i(glGetUniformLocation(program, "grassAO"), 1);

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, grassRoughness);
        glUniform1i(glGetUniformLocation(program, "grassRoughness"), 2);

        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, grassNormal);
        glUniform1i(glGetUniformLocation(program, "grassNormal"), 3);

        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, grassDisplacement);
        glUniform1i(glGetUniformLocation(program, "grassDisplacement"), 4);


        glActiveTexture(GL_TEXTURE5);
        glBindTexture(GL_TEXTURE_2D, rockColor);
        glUniform1i(glGetUniformLocation(program, "rockColor"), 5);

        glActiveTexture(GL_TEXTURE6);
        glBindTexture(GL_TEXTURE_2D, rockAO);
        glUniform1i(glGetUniformLocation(program, "rockAO"), 6);

        glActiveTexture(GL_TEXTURE7);
        glBindTexture(GL_TEXTURE_2D, rockRoughness);
        glUniform1i(glGetUniformLocation(program, "rockRoughness"), 7);

        glActiveTexture(GL_TEXTURE8);
        glBindTexture(GL_TEXTURE_2D, rockNormal);
        glUniform1i(glGetUniformLocation(program, "rockNormal"), 8);

        glActiveTexture(GL_TEXTURE9);
        glBindTexture(GL_TEXTURE_2D, rockDisplacement);
        glUniform1i(glGetUniformLocation(program, "rockDisplacement"), 9);


        glActiveTexture(GL_TEXTURE10);
        glBindTexture(GL_TEXTURE_2D, snowColor);
        glUniform1i(glGetUniformLocation(program, "snowColor"), 10);

        glActiveTexture(GL_TEXTURE11);
        glBindTexture(GL_TEXTURE_2D, snowAO);
        glUniform1i(glGetUniformLocation(program, "snowAO"), 11);

        glActiveTexture(GL_TEXTURE12);
        glBindTexture(GL_TEXTURE_2D, snowRoughness);
        glUniform1i(glGetUniformLocation(program, "snowRoughness"), 12);

        glActiveTexture(GL_TEXTURE13);
        glBindTexture(GL_TEXTURE_2D, snowNormal);
        glUniform1i(glGetUniformLocation(program, "snowNormal"), 13);

        glActiveTexture(GL_TEXTURE14);
        glBindTexture(GL_TEXTURE_2D, snowDisplacement);
        glUniform1i(glGetUniformLocation(program, "snowDisplacement"), 14);



        glBindVertexArray(terrainVAO);
        //        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        for (unsigned strip = 0; strip < numStrips; strip++)
        {

            glDrawElements(GL_TRIANGLE_STRIP,   // primitive type
                numTrisPerStrip + 2,   // number of indices to render
                GL_UNSIGNED_INT,     // index data type
                (void*)(sizeof(unsigned) * (numTrisPerStrip + 2) * strip)); // offset to starting index



        } glUseProgram(0);
    }


    void shutdownTerrain() {
        glDeleteVertexArrays(1, &terrainVAO);
        glDeleteBuffers(1, &terrainVBO);
        glDeleteBuffers(1, &terrainIBO);
    }
}

