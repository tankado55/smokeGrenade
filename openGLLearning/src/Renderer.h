#pragma once

#include <GL/glew.h>
#include "VertexArray.h"
#include "IndexBuffer.h"
#include "Shader.h"

const unsigned int SCR_WIDTH = 960;
const unsigned int SCR_HEIGHT = 540;

#define ASSERT(x) if (!(x)) __debugbreak();
#define GLCall(x) GLClearError();\
    x;\
    ASSERT(GLLogCall(#x, __FILE__, __LINE__)) //#x is the name of the function

void GLClearError();
bool GLLogCall(const char* function, const char* file, int line);

class Renderer
{
public:
    void Clear() const;
    void Draw(const VertexArray& va, const IndexBuffer& ib, const Shader& shader);
    void DrawInstanced(const VertexArray& va, const IndexBuffer& ib, const Shader& shader, int instanceCount);
};