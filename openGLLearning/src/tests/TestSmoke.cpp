#include "TestSmoke.h"
#include "Renderer.h"
#include "imgui.h"
#include "glm/glm.hpp"
#include "InputManager.h"
#include "TimeUpdater.h"



#include <glm/gtc/matrix_transform.hpp>
#include <cmath>
#include <algorithm>

void renderQuad();


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
    m_Proj(glm::perspective(glm::radians(45.0f), float(SCR_WIDTH) / float(SCR_HEIGHT), 0.1f, 500.0f)),
    m_View(glm::translate(glm::mat4(1.0f), glm::vec3(0.0, 0.0, 0.0))),
    m_TranslationA(glm::vec3(0, 0, 0)),
    m_TextureGridMode(glm::vec3(1.2f, 1.0f, 2.0f)),
    m_XCount(30),
    m_YCount(10),
    m_ZCount(30),
    m_Distance(0.5f),
    m_SmokeColor(glm::vec3(0.05, 0.05, 0.05)),
    m_DirLightCol(glm::vec3(0.65, 0.6, 0.4)),
    m_StepSize(0.05),
    m_LigthStepSize(0.25),
    m_DebugVoxels(false),
    m_VolumeDensity(4.0),
    m_ShadowDensity(2.5),
    m_AbsorptionCoefficient(0.5),
    m_ScatteringCoefficient(2.6),
    m_DensityFalloff(0.25)

{
    m_Plane = std::make_unique<Model>("res/models/plane/plane.obj");
    m_PrototypeTexture = std::make_unique<Texture>("res/textures/tex_3.png");
    m_Plane->AddTexture(*m_PrototypeTexture, "texture_diffuse", 0);
    
    m_QuadShader = std::make_unique<Shader>("res/shaders/QuadShader.hlsl");
    m_Quad = std::make_unique<Quad>();

    m_NoiseComputeShader = std::make_unique<ComputeShader>("res/shaders/noise.hlsl");
    m_Noise3DTex = std::make_unique<Texture3D>(128, 128, 128);

    m_DepthShader = std::make_unique<Shader>("res/shaders/simpleDepthShader.hlsl");
    m_DebugDepthQuadShader = std::make_unique<Shader>("res/shaders/debugDepthQuad.hlsl");
    m_DepthFB = std::make_unique<DepthMapFB>();
    m_depthMap = std::make_unique<DepthTexture>();
    m_DepthFB->attachTexture(*m_depthMap);


    // Compute
    m_NoiseComputeShader->Bind();
    m_NoiseComputeShader->SetUniform1i("_Octaves", 6);
    m_NoiseComputeShader->SetUniform1i("_CellSize", 32);
    m_NoiseComputeShader->SetUniform1i("_AxisCellCount", 4);
    m_NoiseComputeShader->SetUniform1f("_Amplitude", 0.62);
    m_NoiseComputeShader->SetUniform1f("_Warp", 0.76);
    m_NoiseComputeShader->SetUniform1f("_Add", 0);
    m_NoiseComputeShader->SetUniform1i("_Seed", 0);
    glDispatchCompute((unsigned int)128 / 8, (unsigned int)128 / 8, (unsigned int)128 / 8);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    

    // Obstacle1
    m_Obstacle = std::make_unique<Model>("res/models/cube/cube.obj");
    m_ObstacleTexture = std::make_unique<Texture>("res/textures/tex_6.png");
    m_Obstacle->AddTexture(*m_ObstacleTexture, "texture_diffuse", 0);
    m_Obstacle->modelMatrix = glm::translate(m_Obstacle->modelMatrix, glm::vec3(5.0, 0.5, 3.0));
    m_Obstacle->modelMatrix = glm::scale(m_Obstacle->modelMatrix, glm::vec3(5.99, 2.99, 0.99));
    // Obstacle2
    m_Obstacle2 = std::make_unique<Model>("res/models/cube/cube.obj");
    m_Obstacle2->AddTexture(*m_ObstacleTexture, "texture_diffuse", 0);
    m_Obstacle2->modelMatrix = glm::translate(m_Obstacle2->modelMatrix, glm::vec3(5.0, 0.5, 6.0));
    m_Obstacle2->modelMatrix = glm::scale(m_Obstacle2->modelMatrix, glm::vec3(5.99, 2.99, 0.99));
    // Obstacle3
    m_Obstacle3 = std::make_unique<Model>("res/models/cube/cube.obj");
    m_Obstacle3->AddTexture(*m_ObstacleTexture, "texture_diffuse", 0);
    glm::vec3 transl3(-5.5, 0.0, -7.0);
    m_Obstacle3->translate(transl3);
    m_Obstacle3->modelMatrix = glm::scale(m_Obstacle3->modelMatrix, glm::vec3(0.99, 5.99, 0.99));
    m_Obs3Left = std::make_unique<Model>("res/models/cube/cube.obj");
    m_Obs3Right = std::make_unique<Model>("res/models/cube/cube.obj");
    m_Obs3Up = std::make_unique<Model>("res/models/cube/cube.obj");
    m_Obs3Down = std::make_unique<Model>("res/models/cube/cube.obj");
    m_Obs3Left->AddTexture(*m_ObstacleTexture, "texture_diffuse", 0);
    m_Obs3Right->AddTexture(*m_ObstacleTexture, "texture_diffuse", 0);
    m_Obs3Up->AddTexture(*m_ObstacleTexture, "texture_diffuse", 0);
    m_Obs3Down->AddTexture(*m_ObstacleTexture, "texture_diffuse", 0);
    m_Obs3Left->translate(transl3 + glm::vec3(-3.0, 0.0, 0.0));
    m_Obs3Left->scale(glm::vec3(0.99, 3.99, 6.99));
    m_Obs3Right->translate(transl3 + glm::vec3(3.0, 0.0, 0.0));
    m_Obs3Right->scale(glm::vec3(0.99, 3.99, 6.99));
    m_Obs3Up->translate(transl3 + glm::vec3(0.0, 0.0, -3.00));
    m_Obs3Up->scale(glm::vec3(4.98, 3.99, 0.99));
    m_Obs3Down->translate(transl3 + glm::vec3(0.0, 0.0, 3.00));
    m_Obs3Down->scale(glm::vec3(4.98, 3.99, 0.99));

    std::vector<Model*> sceneModels;
    sceneModels.push_back(m_Obstacle.get());
    sceneModels.push_back(m_Obstacle2.get());
    sceneModels.push_back(m_Obstacle3.get());
    sceneModels.push_back(m_Obs3Left.get());
    sceneModels.push_back(m_Obs3Right.get());
    sceneModels.push_back(m_Obs3Up.get());
    sceneModels.push_back(m_Obs3Down.get());

    float positions[] = {
        // pos, norma, uv
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
     0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
    -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

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
     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
     0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
     0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
     0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
    -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f
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

    m_VoxelGrid = std::make_unique<VoxelGrid>();

    m_SmokeShader = std::make_unique<Shader>("res/shaders/SmokeShader.hlsl");
    m_VoxelDebugShader = std::make_unique<Shader>("res/shaders/voxelDebugShader.hlsl");
    m_VoxelDebugShader->Bind();
    m_VoxelDebugShader->SetUniform1i("u_XCount", m_VoxelGrid->GetResolution().x);
    m_VoxelDebugShader->SetUniform1i("u_YCount", m_VoxelGrid->GetResolution().y);
    m_VoxelDebugShader->SetUniform1i("u_ZCount", m_VoxelGrid->GetResolution().z);
    m_PlaneShader = std::make_unique<Shader>("res/shaders/BasicPlaneShader.hlsl");
    m_ObstacleShader = std::make_unique<Shader>("res/shaders/ObstacleShader.hlsl");
    m_SmokeShader->Bind();

    m_SmokeShader->SetUniform1i("u_XCount", m_VoxelGrid->GetResolution().x);
    m_SmokeShader->SetUniform1i("u_YCount", m_VoxelGrid->GetResolution().y);
    m_SmokeShader->SetUniform1i("u_ZCount", m_VoxelGrid->GetResolution().z);
    m_VoxelGrid->Bake(sceneModels);

    m_FlatColorShader = std::make_unique<Shader>("res/shaders/flatColorShader.hlsl");

    m_Camera = std::make_unique<Camera>();
    InputManager::GetInstance()->Start(m_Camera.get());
    m_Smoke = std::make_unique<SmokeGrenade>();

    //glEnable(GL_CULL_FACE);
    m_NoiseDebugShader = std::make_unique<Shader>("res/shaders/noiseDebugShader.hlsl");
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

    UpdateInputs(Timem::deltaTime);

    glm::vec3 planeNormal = { 0, 1, 0 };
    glm::vec3 pointOnPlane = { 1, 0, 1 };
    glm::vec3 cameraFront = m_Camera->GetFront();
    glm::vec3 cameraPosition = m_Camera->GetPos();

    glm::vec3 intersectInPlane = rayPlaneIntersection(cameraPosition, cameraFront, pointOnPlane, planeNormal);

    //Depth
    m_DepthShader->Bind();
    m_DepthShader->SetUniformMat4f("u_Proj", m_Proj);
    m_DepthShader->SetUniformMat4f("u_View", m_View);
    m_DepthFB->bind();
    m_DepthFB->clear();
    // draw obstacles depth
    m_DepthShader->SetUniformMat4f("u_Model", m_Obstacle->GetModelMatrix());
    m_Obstacle->Draw(*m_DepthShader);
    m_DepthShader->SetUniformMat4f("u_Model", m_Obstacle2->GetModelMatrix());
    m_Obstacle2->Draw(*m_DepthShader);
    m_DepthShader->SetUniformMat4f("u_Model", m_Obstacle3->GetModelMatrix());
    m_Obstacle3->Draw(*m_DepthShader);
    m_DepthShader->SetUniformMat4f("u_Model", m_Obs3Left->GetModelMatrix());
    m_Obs3Left->Draw(*m_DepthShader);
    m_DepthShader->SetUniformMat4f("u_Model", m_Obs3Right->GetModelMatrix());
    m_Obs3Right->Draw(*m_DepthShader);
    m_DepthShader->SetUniformMat4f("u_Model", m_Obs3Up->GetModelMatrix());
    m_Obs3Up->Draw(*m_DepthShader);
    m_DepthShader->SetUniformMat4f("u_Model", m_Obs3Down->GetModelMatrix());
    m_Obs3Down->Draw(*m_DepthShader);
    // unnecessary plane depth
    //m_DepthShader->SetUniformMat4f("u_Model", glm::scale(glm::mat4(1.0f), glm::vec3(1.5, 0.0, 1.5)));
    //m_Plane->Draw(*m_DepthShader);
    m_DepthFB->unBind();
    // reset viewport
    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

    { // Plane
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::scale(model, glm::vec3(1.5, 0.0, 1.5));
        m_PlaneShader->Bind();
        m_PlaneShader->SetUniformMat4f("u_View", m_View);
        m_PlaneShader->SetUniformMat4f("u_Projection", m_Proj);
        m_PlaneShader->SetUniformMat4f("u_Model", model);

        m_Plane->Draw(*m_PlaneShader);
    }

    { // intersection cube
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, intersectInPlane);
        model = glm::scale(model, glm::vec3(0.25, 0.25, 0.25));
        m_FlatColorShader->Bind(); // it is done also in renderer.draw but it is necessary here to set the uniform
        m_FlatColorShader->SetUniformMat4f("u_View", m_View);
        m_FlatColorShader->SetUniformMat4f("u_Projection", m_Proj);
        m_FlatColorShader->SetUniformMat4f("u_Model", model);

        m_FlatColorShader->SetUniform4f("color", 0.7,0.0,0.0,1.0);
        renderer.Draw(*m_VAO, *m_IndexBuffer, *m_FlatColorShader);
    }

    { // obstacles
        m_ObstacleShader->Bind(); // it is done also in renderer.draw but it is necessary here to set the uniform
        m_ObstacleShader->SetUniformMat4f("u_View", m_View);
        m_ObstacleShader->SetUniformMat4f("u_Projection", m_Proj);
        m_ObstacleShader->SetUniformVec3f("viewPos", m_Camera->GetPos());
        m_ObstacleShader->SetUniformMat4f("u_Model", m_Obstacle->GetModelMatrix());
        m_Obstacle->Draw(*m_ObstacleShader);
        m_ObstacleShader->SetUniformMat4f("u_Model", m_Obstacle2->GetModelMatrix());
        m_Obstacle2->Draw(*m_ObstacleShader);
        m_ObstacleShader->SetUniformMat4f("u_Model", m_Obstacle3->GetModelMatrix());
        m_Obstacle3->Draw(*m_ObstacleShader);

        m_ObstacleShader->SetUniformMat4f("u_Model", m_Obs3Left->GetModelMatrix());
        m_Obs3Left->Draw(*m_ObstacleShader);
        m_ObstacleShader->SetUniformMat4f("u_Model", m_Obs3Right->GetModelMatrix());
        m_Obs3Right->Draw(*m_ObstacleShader);
        m_ObstacleShader->SetUniformMat4f("u_Model", m_Obs3Up->GetModelMatrix());
        m_Obs3Up->Draw(*m_ObstacleShader);
        m_ObstacleShader->SetUniformMat4f("u_Model", m_Obs3Down->GetModelMatrix());
        m_Obs3Down->Draw(*m_ObstacleShader);
        
    }
    
    //{ //smoke
    //    double toPass = Timem::deltaTime;
    //    m_Smoke->Update(toPass);
    //    m_Smoke->Draw(*m_SmokeShader); // it only set the uniforms
    //    glm::mat4 model = glm::mat4(1.0);
    //    model = glm::scale(model, glm::vec3(m_VoxelGrid->voxelSize, m_VoxelGrid->voxelSize, m_VoxelGrid->voxelSize));
    //    model = model * m_VoxelGrid->modelMatrix;
    //    m_SmokeShader->Bind(); // it is done also in renderer.draw but it is necessary here to set the uniform
    //    m_SmokeShader->SetUniformMat4f("u_Model", model);
    //    m_SmokeShader->SetUniformMat4f("u_View", m_View);
    //    m_SmokeShader->SetUniformMat4f("u_Projection", m_Proj);
    //    m_SmokeShader->SetUniformVec3f("explosionPos", intersectInPlane);
    //    
    //
    //    renderer.DrawInstanced(
    //        *m_VAO,
    //        *m_IndexBuffer,
    //        *m_SmokeShader,
    //        m_VoxelGrid->voxelCount
    //    );
    //}

    if (m_DebugVoxels)
    { //voxel debugging
        m_VoxelGrid->BindBufferToTexture(*m_VoxelDebugShader); // it only set the uniforms
        glm::mat4 model = glm::mat4(1.0);
        model = glm::scale(model, glm::vec3(m_VoxelGrid->GetVoxelSize()));
        model = model * m_VoxelGrid->modelMatrix;
        m_VoxelDebugShader->Bind(); // it is done also in renderer.draw but it is necessary here to set the uniform
        m_VoxelDebugShader->SetUniformMat4f("u_Model", model);
        m_VoxelDebugShader->SetUniformMat4f("u_View", m_View);
        m_VoxelDebugShader->SetUniformMat4f("u_Projection", m_Proj);
    
        renderer.DrawInstanced(
            *m_VAO,
            *m_IndexBuffer,
            *m_VoxelDebugShader,
            m_VoxelGrid->GetVoxelCount()
        );
    }

    //glDepthMask(GL_FALSE);
    glDisable(GL_DEPTH_TEST);
    {
        m_QuadShader->Bind();
        m_Noise3DTex->Bind(0);
        m_QuadShader->SetUniform1i("_NoiseTex", 0);
        m_depthMap->Bind(1);
        m_QuadShader->SetUniform1i("_DepthMap", 1);
        double time = Timem::deltaTime;
        m_Smoke->Update(time);
        m_Smoke->SetUniforms(*m_QuadShader);
        m_QuadShader->SetUniformMat4f("u_Projection", m_Proj);
        m_QuadShader->SetUniformMat4f("u_View", m_View);
        m_QuadShader->SetUniform4f("u_CameraWorldPos", m_Camera->GetPos().x, m_Camera->GetPos().y, m_Camera->GetPos().z, 1.0);
        m_VoxelGrid->BindBufferToTexture(*m_QuadShader);
        m_QuadShader->SetUniformMat4f("toVoxelLocal", m_VoxelGrid->GetToVoxelLocal());
        m_QuadShader->SetUniformVec3f("resolution", m_VoxelGrid->GetResolution());
        m_QuadShader->SetUniform3f("u_DirLight.direction", 0.0,-1.0,0.0);
        m_QuadShader->SetUniformVec3f("u_DirLight.color", m_DirLightCol); //0.38,0.38,0.38
        m_QuadShader->SetUniform1f("u_AbsorptionCoefficient", m_AbsorptionCoefficient);
        m_QuadShader->SetUniform1f("u_ScatteringCoefficient", m_ScatteringCoefficient);
        m_QuadShader->SetUniformVec3f("u_ExtinctionColor", glm::vec3(1.0,1.0,1.0));
        m_QuadShader->SetUniform1f("iTime", glfwGetTime());
        m_QuadShader->SetUniform1f("_DensityFalloff", 1.0 - m_DensityFalloff);
        m_QuadShader->SetUniformVec3f("u_VoxelSpaceBounds", m_VoxelGrid->GetBounds());
        m_QuadShader->SetUniform1f("_SmokeSize", 4.0);
        m_QuadShader->SetUniformVec3f("_AnimationDirection", glm::vec3(0.0, -0.1, 0.0));
        // smoke parameters
        m_QuadShader->SetUniformVec3f("u_SmokeColor", m_SmokeColor);
        m_QuadShader->SetUniform1f("u_StepSize", m_StepSize);
        m_QuadShader->SetUniform1f("u_LigthStepSize", m_LigthStepSize);
        m_QuadShader->SetUniform1f("u_VolumeDensity", m_VolumeDensity);
        m_QuadShader->SetUniform1f("u_ShadowDensity", m_ShadowDensity);
        m_Quad->Draw(*m_QuadShader);
    }
    glEnable(GL_DEPTH_TEST);
    //glDepthMask(GL_TRUE);
    
    //debug shader
    //m_Noise3DTex->Bind();
    //m_NoiseDebugShader->Bind();
    //m_NoiseDebugShader->SetUniform1i("_NoiseTex", 0);
    //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //renderQuad();

    //debug depth
    //m_depthMap->Bind(0);
    //m_DebugDepthQuadShader->Bind();
    //m_DebugDepthQuadShader->SetUniform1f("near_plane", 0.1f);
    //m_DebugDepthQuadShader->SetUniform1f("far_plane", 500.0f);
    //m_DebugDepthQuadShader->SetUniform1i("depthMap", 0);
    //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //renderQuad();
    
}

void Test::TestSmoke::OnImGuiRenderer()
{
    ImGui::SliderFloat3("Smoke Color", &m_SmokeColor.x, 0.0, 1.0);
    ImGui::SliderFloat3("Light Color", &m_DirLightCol.x, 0.0, 1.0);
    ImGui::SliderFloat("StepSize", &m_StepSize, 0.01f, 0.5f);
    ImGui::SliderFloat("LigthStepSize", &m_LigthStepSize, 0.01f, 0.5f);
    ImGui::SliderFloat3("Smoke Size", (float*)m_Smoke->GetEllipsoidPtr(), 1.0f, 4.0f);
    ImGui::SliderFloat("Volume Density", &m_VolumeDensity, 0.0f, 5.0f);
    ImGui::SliderFloat("Shadow Density", &m_ShadowDensity, 0.0f, 5.0f);
    ImGui::SliderFloat("Absorption Coefficient", &m_AbsorptionCoefficient, 0.0f, 5.0f);
    ImGui::SliderFloat("Scattering Coefficient", &m_ScatteringCoefficient, 0.0f, 5.0f);
    ImGui::SliderFloat("Density Falloff", &m_DensityFalloff, 0.0f, 1.0f);
    ImGui::Checkbox("Debug Voxels", &m_DebugVoxels);
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
}

void Test::TestSmoke::UpdateInputs(const double& deltaTime)
{
    GLFWwindow* window = InputManager::GetInstance()->GetWindow();

    int state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT);
    if (state == GLFW_PRESS)
    {
        m_PrevLeftButtonState = m_LeftButtonState;
        m_LeftButtonState = true;
        if (m_PrevLeftButtonState == false && m_LeftButtonState == true)
        {
            glm::vec3 planeNormal = { 0, 1, 0 };
            glm::vec3 pointOnPlane = { 1, 0, 1 };
            glm::vec3 cameraFront = m_Camera->GetFront();
            glm::vec3 cameraPosition = m_Camera->GetPos();

            glm::vec3 intersectInPlane = rayPlaneIntersection(cameraPosition, cameraFront, pointOnPlane, planeNormal);
            m_VoxelGrid->ClearStatus();
            m_Smoke->Detonate(intersectInPlane);
            m_VoxelGrid->Flood(intersectInPlane, *m_Smoke->GetEllipsoidPtr(), 10.0);
        }
    }
    else
    {
        m_PrevLeftButtonState = m_LeftButtonState;
        m_LeftButtonState = false;
    }
}

// renderQuad() renders a 1x1 XY quad in NDC
// -----------------------------------------
unsigned int quadVAO = 0;
unsigned int quadVBO;
void renderQuad()
{
    if (quadVAO == 0)
    {
        float quadVertices[] = {
            // positions        // texture Coords
            -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
             1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
             1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };
        // setup plane VAO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}
