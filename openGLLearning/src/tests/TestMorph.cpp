#include "TestMorph.h"

#include "Renderer.h"
#include "imgui.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "InputManager.h"

Test::TestMorph::TestMorph() :
    m_Proj(glm::perspective(glm::radians(45.0f), 960.0f / 540.0f, 0.1f, 500.0f)),
    m_View(glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, -100.0))),
    m_TranslationA(glm::vec3(0, 0, 0)),
    m_LightPos(glm::vec3(1.2f, 1.0f, 2.0f)),
    m_Interpolation(1.0),
    u_InterpolationSpeed(0.1)
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

    m_Shader = std::make_unique<Shader>("res/shaders/Basic.hlsl");
    m_Shader->Bind();
    m_Shader->SetUniform3f("u_Light.position", m_LightPos.x, m_LightPos.y, m_LightPos.z);
    m_Shader->SetUniform3f("u_Light.ambient", 0.2f, 0.2f, 0.2f);
    m_Shader->SetUniform3f("u_Light.diffuse", 0.5f, 0.5f, 0.5f);
    m_Shader->SetUniform3f("u_Light.specular", 1.0f, 1.0f, 1.0f);
    m_Shader->SetUniform3f("u_Material.ambient", 1.0f, 0.5f, 0.31f);
    m_Shader->SetUniform3f("u_Material.diffuse", 1.0f, 0.5f, 0.31f);
    m_Shader->SetUniform3f("u_Material.specular", 0.5f, 0.5f, 0.5f);
    m_Shader->SetUniform1f("u_Material.shininess", 32.0f);


    m_LightShader = std::make_unique<Shader>("res/shaders/LightSource.hlsl");
    m_LightShader->Bind();
    m_LightShader->SetUniform4f("u_Color", 1.0f, 1.0f, 1.0f, 1.0f);

    //m_Texture = std::make_unique<Texture>("res/textures/logo.png");
    //m_Shader->SetUniform1i("u_Texture", 0); // slot of the texture

    m_Camera = std::make_unique<Camera>();
    //InputManager::GetInstance()->Start(m_Camera.get());


    m_Backpack = std::make_unique<Model>("res/models/backpack/backpack.obj");
    m_BackpackShader = std::make_unique<Shader>("res/shaders/ModelGuitarTest.hlsl");
}

Test::TestMorph::~TestMorph()
{
}

void Test::TestMorph::OnUpdate(float deltaTime, GLFWwindow*& window)
{
    m_Camera->ProcessKeyboardInput(deltaTime, window);
    ProcessKeyboardInput(deltaTime, window);
    //m_Camera->ProcessMouseInput();
    m_View = m_Camera->GetView();
}

void Test::TestMorph::OnRenderer()
{
    Renderer renderer; //that's ok because Rendered does not have internal state

    //m_Texture->Bind(); // di default in slot 0
    /*
    {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, m_TranslationA);
        glm::mat4 mvp = m_Proj * m_View * model;
        m_Shader->Bind(); // it is done also in renderer.draw but it is necessary here to set the uniform
        m_Shader->SetUniformMat4f("u_Model", model);
        m_Shader->SetUniformMat4f("u_MVP", mvp);
        m_Shader->SetUniformVec3f("u_ViewPosition", m_Camera->GetPos());

        renderer.Draw(*m_VAO, *m_IndexBuffer, *m_Shader);
    }
    {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, m_LightPos);
        model = glm::scale(model, glm::vec3(0.2f));
        glm::mat4 mvp = m_Proj * m_View * model;
        m_LightShader->Bind(); // it is done also in renderer.draw but it is necessary here to set the uniform
        m_LightShader->SetUniformMat4f("u_MVP", mvp);

        renderer.Draw(*m_VAO, *m_IndexBuffer, *m_Shader);
        renderer.Draw(*m_VAO, *m_IndexBuffer, *m_LightShader);
    }
    */
    
    {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-2.0, 0, 0));
        glm::mat4 mvp = m_Proj * m_View * model;
        m_BackpackShader->Bind(); // it is done also in renderer.draw but it is necessary here to set the uniform
        m_BackpackShader->SetUniformMat4f("u_MVP", mvp);
        m_BackpackShader->SetUniform1f("u_Interpolation", m_Interpolation);
        m_Backpack->Draw(*m_BackpackShader);
    }
    
}

void Test::TestMorph::OnImGuiRenderer()
{
    ImGui::SliderFloat3("Translation A", &m_TranslationA.x, -960.0f/2.0f, 960.0f/2.0f);
    ImGui::SliderFloat3("Translation B", &m_LightPos.x, -10.0f, 10.0f);
    ImGui::SliderFloat("Interpolation", &m_Interpolation, 0.0f, 1.0f);
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
}

void Test::TestMorph::ProcessKeyboardInput(float deltaTime, GLFWwindow* window)
{
    float speed = u_InterpolationSpeed * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        m_Interpolation -= speed;
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        m_Interpolation += speed;

    m_Interpolation = std::max(0.0f, std::min(m_Interpolation, 1.0f));
}
