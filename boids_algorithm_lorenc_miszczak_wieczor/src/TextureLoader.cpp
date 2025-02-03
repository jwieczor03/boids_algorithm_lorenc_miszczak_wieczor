#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"  // Œcie¿ka do pliku stb_image.h

#include <GL/glew.h>
#include <vector>
#include <string>
#include <unordered_map>

GLuint loadCubemap(const std::vector<std::string>& faces) {
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (GLuint i = 0; i < faces.size(); i++) {
        unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data) {

            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else {
            fprintf(stderr, "Failed to load cubemap texture: %s\n", faces[i].c_str());
            stbi_image_free(data);
            return 0;
        }
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}
//GLuint loadTextures(const std::unordered_map<std::string, std::string>& textureFiles,
//    std::unordered_map<std::string, GLuint>& textureIDs) {
//    int width, height, nrChannels;
//
//    for (const auto& [name, path] : textureFiles) {
//        GLuint textureID;
//        glGenTextures(1, &textureID);
//        glBindTexture(GL_TEXTURE_2D, textureID);
//
//        unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);
//        if (data) {
//            GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
//            glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
//            glGenerateMipmap(GL_TEXTURE_2D);
//
//            // Ustawienia filtrowania i wrapowania
//            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
//            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
//            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
//
//            stbi_image_free(data);
//            textureIDs[name] = textureID;
//        }
//        else {
//            fprintf(stderr, "Failed to load cubemap texture: %s\n", path.c_str());
//            stbi_image_free(data);
//            return 0;  // Zwracamy 0 w przypadku b³êdu
//        }
//    }
//
//    return 1;  // Sukces
//}