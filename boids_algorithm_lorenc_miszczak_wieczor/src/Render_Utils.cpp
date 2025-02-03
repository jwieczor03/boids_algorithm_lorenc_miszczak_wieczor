#include "Headers/Render_Utils.h"

#include <algorithm>

#include "GL/glew.h"
#include "freeglut.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>



void Core::RenderContext::initFromAssimpMesh(aiMesh* mesh) {
    vertexArray = 0;
    vertexBuffer = 0;
    vertexIndexBuffer = 0;

    std::vector<aiVector2D> textureCoord;
    std::vector<unsigned int> indices;
    //tex coord must be converted to 2d vecs
    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        if (mesh->mTextureCoords[0] != nullptr) {
            //aiVector2D()
            textureCoord.push_back(aiVector2D(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y));
        }
        else {
            textureCoord.push_back(aiVector2D(0.0f, 0.0f));
        }
    }
    if (mesh->mTextureCoords[0] == nullptr) {
        std::cout << "no uv coords\n";
    }
    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        // retrieve all indices of the face and store them in the indices vector
        for (unsigned int j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
    }




}

void Core::DrawVertexArray(const float* vertexArray, int numVertices, int elementSize)
{
    glVertexAttribPointer(0, elementSize, GL_FLOAT, false, 0, vertexArray);
    glEnableVertexAttribArray(0);

    glDrawArrays(GL_TRIANGLES, 0, numVertices);
}

void Core::DrawVertexArrayIndexed(const float* vertexArray, const int* indexArray, int numIndexes, int elementSize)
{
    glVertexAttribPointer(0, elementSize, GL_FLOAT, false, 0, vertexArray);
    glEnableVertexAttribArray(0);

    glDrawElements(GL_TRIANGLES, numIndexes, GL_UNSIGNED_INT, indexArray);
}


void Core::DrawVertexArray(const VertexData& data)
{
    int numAttribs = std::min(VertexData::MAX_ATTRIBS, data.NumActiveAttribs);
    for (int i = 0; i < numAttribs; i++)
    {
        glVertexAttribPointer(i, data.Attribs[i].Size, GL_FLOAT, false, 0, data.Attribs[i].Pointer);
        glEnableVertexAttribArray(i);
    }
    glDrawArrays(GL_TRIANGLES, 0, data.NumVertices);
}

void Core::DrawContext(Core::RenderContext& context)
{

    glBindVertexArray(context.vertexArray);
    glDrawElements(
        GL_TRIANGLES,      // mode
        context.size,    // count
        GL_UNSIGNED_INT,   // type
        (void*)0           // element array buffer offset
    );
    glBindVertexArray(0);
}
