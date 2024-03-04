#include "TestSmoke.h"

#include "Renderer.h"
#include "imgui.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "InputManager.h"
#include <cmath>

glm::vec3 rayPlaneIntersection(glm::vec3 rayOrigin, glm::vec3 rayDirection, glm::vec3 planePoint, glm::vec3 planeNormal) {
    float denominator = glm::dot(planeNormal, rayDirection);

    if (denominator == 0) {
        // Ray is parallel to the plane
        return { 0, 0, 0 };
    }

    float t = ((planePoint.x - rayOrigin.x) * planeNormal.x +
        (planePoint.y - rayOrigin.y) * planeNormal.y +
        (planePoint.z - rayOrigin.z) * planeNormal.z) / denominator;

    glm::vec3 intersectionPoint;
    intersectionPoint.x = rayOrigin.x + t * rayDirection.x;
    intersectionPoint.y = rayOrigin.y + t * rayDirection.y;
    intersectionPoint.z = rayOrigin.z + t * rayDirection.z;

    return intersectionPoint;
}


Test::TestSmoke::TestSmoke() :
    m_Proj(glm::perspective(glm::radians(45.0f), 960.0f / 540.0f, 0.1f, 500.0f)),
    m_View(glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, -100.0))),
    m_TranslationA(glm::vec3(0, 0, 0)),
    m_TextureGridMode(glm::vec3(1.2f, 1.0f, 2.0f)),
    m_XCount(30),
    m_YCount(10),
    m_ZCount(30),
    m_Distance(0.5f)
{
    float positions[] = {
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
     0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
    -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,

    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
     0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
     0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
     0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
    -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
    };

    unsigned int indices[36];
    for (int i = 0; i < 36; i++)
    {
        indices[i] = i;
    }

    GLCall(glEnable(GL_BLEND));
    GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)); // it defines how opengl blend alpha pixels

    m_VAO = std::make_unique<VertexArray>();

    m_VertexBuffer = std::make_unique<VertexBuffer>(positions, 36 * 6 * sizeof(float));
    VertexBufferLayout layout;
    layout.Push<float>(3);
    layout.Push<float>(3);
    m_VAO->AddBuffer(*m_VertexBuffer, layout);

    m_IndexBuffer = std::make_unique<IndexBuffer>(indices, 36);

    m_SmokeShader = std::make_unique<Shader>("res/shaders/SmokeShader.hlsl");
    m_SmokeShader->Bind();

    m_SmokeShader->SetUniform1i("u_XCount", m_XCount);
    m_SmokeShader->SetUniform1i("u_YCount", m_YCount);
    m_SmokeShader->SetUniform1i("u_ZCount", m_ZCount);
    m_SmokeShader->SetUniform1f("u_VoxelSize", m_Distance);

    m_WhiteShader = std::make_unique<Shader>("res/shaders/WhiteSingleShader.hlsl");

    m_Camera = std::make_unique<Camera>();
    InputManager::GetInstance()->Start(m_Camera.get());
}

Test::TestSmoke::~TestSmoke()
{
}

void Test::TestSmoke::OnUpdate(float deltaTime, GLFWwindow*& window)
{
    m_Camera->ProcessKeyboardInput(deltaTime, window);
    //m_Camera->ProcessMouseInput();
    m_View = m_Camera->GetView();
}

void Test::TestSmoke::OnRenderer()
{
    Renderer renderer; //that's ok because Rendered does not have internal state

    glm::vec3 planeNormal = { 0, 1, 0 };
    glm::vec3 pointOnPlane = { 1, 0, 1 };
    glm::vec3 cameraFront = m_Camera->GetFront();
    glm::vec3 cameraPosition = m_Camera->GetPos();

    glm::vec3 intersectInPlane = rayPlaneIntersection(cameraPosition, cameraFront, pointOnPlane, planeNormal);

     {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, intersectInPlane);
        //model = glm::scale(model, glm::vec3(0.5, 0.5, 0.5));
        m_WhiteShader->Bind(); // it is done also in renderer.draw but it is necessary here to set the uniform
        m_WhiteShader->SetUniformMat4f("u_View", m_View);
        m_WhiteShader->SetUniformMat4f("u_Projection", m_Proj);
        m_WhiteShader->SetUniformMat4f("u_Model", model);

        renderer.Draw(*m_VAO, *m_IndexBuffer, *m_WhiteShader);
    }
    
 
    { // cube in the origin
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, m_TranslationA);
        m_WhiteShader->Bind(); // it is done also in renderer.draw but it is necessary here to set the uniform
        m_WhiteShader->SetUniformMat4f("u_View", m_View);
        m_WhiteShader->SetUniformMat4f("u_Projection", m_Proj);
        m_WhiteShader->SetUniformMat4f("u_Model", model);

        renderer.Draw(*m_VAO, *m_IndexBuffer, *m_WhiteShader);
    }
    
    { //batch
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, m_TranslationA);
        //model = glm::scale(model, glm::vec3(0.5,0.5,0.5));
        glm::mat4 mvp = m_Proj * m_View * model;
        m_SmokeShader->Bind(); // it is done also in renderer.draw but it is necessary here to set the uniform
        m_SmokeShader->SetUniformMat4f("u_MVP", mvp);

        renderer.DrawInstanced(*m_VAO, *m_IndexBuffer, *m_SmokeShader, m_XCount * m_YCount * m_ZCount);
    }
    
    
}

void Test::TestSmoke::OnImGuiRenderer()
{
    ImGui::SliderFloat3("Translation A", &m_TranslationA.x, -960.0f/2.0f, 960.0f/2.0f);
    ImGui::SliderFloat3("Translation B", &m_TextureGridMode.x, -10.0f, 10.0f);
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
}
