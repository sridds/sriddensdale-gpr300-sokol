#include "scene.h"

// imgui
#include "imgui/imgui.h"
#include "imguizmo/imguizmo.h"
#include <iostream>

// glm
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"

// batteries
#include "batteries/opengl.h"

struct FullscreenQuad{
    GLuint vao;
    GLuint vbo;

    void Init(){
        // position (x, y), tex coord (u, v)
        float verts[] = {
            // triangle 1
            -1.0f, 1.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f,
            1.0f, -1.0f, 1.0f, 0.0f,

            // triangle 2
            -1.0f, 1.0f, 0.0f, 1.0f,
            1.0f, -1.0f, 1.0f, 0.0f,
            1.0f, 1.0f, 1.0f, 1.0f
        };

        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);

        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);

        glBufferData(GL_ARRAY_BUFFER, sizeof(verts), &verts, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(sizeof(float) * 2));

        glBindVertexArray(0);
    }
};

FullscreenQuad quad;
int SCREEN_WIDTH = 800;
int SCREEN_HEIGHT = 600;
int SHADOW_RESOLUTION = 1024;

float minBias = 0.005f;
float maxBias = 0.05f;

Scene::Scene()
{
    // meshes
    suzanne = std::make_unique<ew::Model>("assets/models/suzanne.obj");
    plane = ew::Mesh(ew::createPlane(10, 10, 5));

    // shaders
    blinnphong = std::make_unique<ew::Shader>("assets/shaders/default.vs", "assets/shaders/default.fs");
    simpleDepthShader = std::make_unique<ew::Shader>("assets/shaders/simpleDepth.vs", "assets/shaders/simpleDepth.fs");
    fullscreen = std::make_unique<ew::Shader>("assets/shaders/fullscreen.vs", "assets/shaders/fullscreen.fs");
    
    // texture
    rockColorTexture = std::make_unique<ew::Texture>("assets/textures/rock_color.jpg");

    // quad
    quad.Init();

    // initialize light camera
    lightCam.orthographic = true;
    lightCam.aspectRatio = 1.0f;
    lightCam.orthoHeight = 10.0f;
    lightCam.nearPlane = 0.01f;
    lightCam.farPlane = 50.0f;
    lightCam.target = glm::vec3(0.0f);

    // set up frame buffer
    glCreateFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    {
        glGenTextures(1, &fboTexture);
        glBindTexture(GL_TEXTURE_2D, fboTexture);

        // creates an 800 x 600 render texture with 8 bytes (unsigned)
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fboTexture, 0);

        // create depth texture
        glGenTextures(1, &fboDepth);
        glBindTexture(GL_TEXTURE_2D, fboDepth);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, fboDepth, 0);
    }

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fboTexture, 0);

    // set up depth buffer
    glCreateFramebuffers(1, &depthFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, depthFBO);
    {
        // initialize depth texture
        glGenTextures(1, &depthTexture);
        glBindTexture(GL_TEXTURE_2D, depthTexture);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, 1024, 1024, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);

        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
    }

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
        std::cout << "Framebuffer is not complete" << std::endl;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

Scene::~Scene()
{
    glDeleteFramebuffers(1, &fbo);
}

void Scene::Update(float dt)
{
    batteries::Scene::Update(dt);

    /* body */
}

auto matrix = glm::mat4(1.0f);

void Scene::Render(void)
{
    lightCam.position = glm::vec3(lightPos[0], lightPos[1], lightPos[2]);
    lightCam.target = glm::vec3(0.0f);

    // shadows
    glViewport(0, 0, SHADOW_RESOLUTION, SHADOW_RESOLUTION);
    glBindFramebuffer(GL_FRAMEBUFFER, depthFBO);
    {
        // light space matrix
        lightSpaceMatrix = lightCam.projectionMatrix() * lightCam.viewMatrix();

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_DEPTH_BUFFER_BIT);

        simpleDepthShader->use();
        simpleDepthShader->setMat4("lightSpaceMatrix", lightSpaceMatrix);

        glEnable(GL_DEPTH_TEST);
        simpleDepthShader->setMat4("model", matrix);

        // render scene
        suzanne->draw();
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

    // Suzanne
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    {
        const auto view_proj = camera.Projection() * camera.View();

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glEnable(GL_DEPTH_TEST);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, rockColorTexture->getID()); 

        blinnphong->use();
        blinnphong->setInt("texture0", 0);

        // scene matrices
        blinnphong->setMat4("model", matrix);
        blinnphong->setMat4("view_proj", view_proj);
        blinnphong->setVec3("camera_position", camera.position);
        
        blinnphong->setVec3("material.ambient", {ambient, ambient, ambient});
        blinnphong->setVec3("material.diffuse", {diffuse, diffuse, diffuse});
        blinnphong->setVec3("material.specular", {specular, specular, specular});
        blinnphong->setFloat("material.shininess", shininess);

        blinnphong->setFloat("ambient.intensity", 1.0f);
        blinnphong->setVec3("ambient.color", {0.5f, 0.5f, 0.5f});

        blinnphong->setVec3("light.color", {0.5f, 0.5f, 0.5f});
        blinnphong->setVec3("light.position", {lightPos[0], lightPos[1], lightPos[2]});

        blinnphong->setMat4("lightSpaceMatrix", lightSpaceMatrix);
        blinnphong->setInt("shadowMap", 1);

        blinnphong->setFloat("minBias", minBias);
        blinnphong->setFloat("maxBias", maxBias);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, depthTexture);

        blinnphong->setMat4("model", matrix);
        suzanne->draw();

        glm::mat4 planeModel = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
        blinnphong->setMat4("model", planeModel);
        plane.draw();
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Post processing
    {
        fullscreen->use();

        glDisable(GL_DEPTH_TEST);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glBindVertexArray(quad.vao);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, fboTexture);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
}

void Scene::Debug(void)
{
    ImGuizmo::BeginFrame();
    ImGuizmo::SetDrawlist(ImGui::GetBackgroundDrawList());
    ImGuizmo::SetRect(0, 0, ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y);

    glm::mat4 m{1.0f};
    auto *view = glm::value_ptr(camera.View());
    auto *proj = glm::value_ptr(camera.Projection());
    
    ImGuizmo::DrawGrid(view, proj, glm::value_ptr(m), 100.0f);

    ImGuizmo::Manipulate(
        view,
        proj,
        ImGuizmo::ROTATE,
        ImGuizmo::WORLD,
        glm::value_ptr(matrix)
    );

    cameracontroller.Debug();

    ImGui::Begin("Controlls", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

    ImGui::Checkbox("Paused", &time.paused);
    ImGui::SliderFloat("Time Factor", &time.factor, 0.0f, 10.0f);
    ImGui::SliderFloat("Ambient", &ambient, 0.0f, 1.0f);
    ImGui::SliderFloat("Diffuse", &diffuse, 0.0f, 1.0f);
    ImGui::SliderFloat("Specular", &specular, 0.0f, 1.0f);
    ImGui::SliderFloat("Shininess", &shininess, 0.0f, 1.0f);
    ImGui::SliderFloat3("Light Position", &lightPos[0], -5.0f, 5.0f);

    ImGui::SliderFloat("Min Shadow Bias", &minBias, 0.0, 1.0);
    ImGui::SliderFloat("Max Shadow Bias", &maxBias, 0.0, 1.0);

    ImGui::Image((void*)(intptr_t)fboTexture, ImVec2(400, 300), ImVec2(0, 1), ImVec2(1, 0));
    ImGui::Image((void*)(intptr_t)fboDepth, ImVec2(400, 300), ImVec2(0, 1), ImVec2(1, 0));
    ImGui::Image((void*)(intptr_t)depthTexture, ImVec2(400, 300), ImVec2(0, 1), ImVec2(1, 0));
    /* build debug ui here */

    ImGui::End();
}