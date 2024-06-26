#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include "Renderer.h"

#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "VertexArray.h"
#include "Shader.h"
#include "VertexBufferLayout.h"
#include "Texture.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include "imgui-1.90.1/imgui.h"
#include "imgui-1.90.1/backends/imgui_impl_opengl3.h"
#include "imgui-1.90.1/backends/imgui_impl_glfw.h"

#include "InputManager.h"


#include "tests/TestSmoke.h"
#include "TimeUpdater.h"


int main(void)
{
    GLFWwindow* window;

    // Print OpenGL version
    int major, minor, revision;
    glfwGetVersion(&major, &minor, &revision);
    printf("OpenGL Version: %d.%d.%d\n", major, minor, revision);

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Smoke Grenade Test Chamber", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    glfwSwapInterval(1); //vsync

    if (glewInit() != GLEW_OK)
        std::cout << "glew ERROR!!!" << std::endl;

    std::cout << glGetString(GL_VERSION) << std::endl;
    // end of glfw related things
    { // thit scope is here in order to call the distructor of vb and ib before the openglcontex is destroyd, in this way there is no infinite loop with GLCall that check for the errors. In this way the app close automatically.
        
        GLCall(glEnable(GL_BLEND));
        GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)); // it defines how opengl blend alpha pixels
        GLCall(glEnable(GL_DEPTH_TEST));
        glDepthFunc(GL_LEQUAL);



        Renderer renderer;

        const char* glsl_version = "#version 130";
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
        ImGui::StyleColorsDark();
        ImGui_ImplGlfw_InitForOpenGL(window, true);
#ifdef __EMSCRIPTEN__
        ImGui_ImplGlfw_InstallEmscriptenCanvasResizeCallback("#canvas");
#endif
        ImGui_ImplOpenGL3_Init(glsl_version);

        Test::Test* currentTest = nullptr;
        Test::TestMenu* testMenu = new Test::TestMenu(currentTest);
        currentTest = testMenu;

        testMenu->RegisterTest<Test::TestSmoke>("Smoke Test");

        float deltaTime = 0.0f;	// Time between current frame and last frame
        float lastFrame = 0.0f; // Time of last frame

        InputManager::GetInstance()->SetWindow(window);

        /* Loop until the user closes the window */
        while (!glfwWindowShouldClose(window))
        {
            Timem::TimeUpdater::UpdateDeltaTime();
            float currentFrame = glfwGetTime();
            deltaTime = currentFrame - lastFrame;
            lastFrame = currentFrame;

            /* Render here */
            GLCall(glClearColor(0.0f, 0.0f, 0.0f, 1.0f));
            renderer.Clear();

            // Start the Dear ImGui frame
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();
            if (currentTest)
            {
                currentTest->OnUpdate(deltaTime, window);
                currentTest->OnRenderer();
                ImGui::Begin("Settings");
                if (currentTest != testMenu && ImGui::Button("<-"))
                {
                    delete currentTest;
                    currentTest = testMenu;
                }
                currentTest->OnImGuiRenderer();
                ImGui::End();
            }
            
            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            glfwSwapBuffers(window);
            glfwPollEvents();
        }
        delete currentTest;
        if (currentTest != testMenu)
        {
            delete testMenu;
        }
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();
    return 0;
}