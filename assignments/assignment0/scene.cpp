#include "scene.h"

// imgui
#include "imgui/imgui.h"
#include "imguizmo/imguizmo.h"

// glm
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"

// batteries
#include "batteries/opengl.h"

Scene::Scene()
{
    suzanne = std::make_unique<ew::Model>("assets/models/suzanne.obj");
    blinnphong = std::make_unique<ew::Shader>("assets/shaders/default.vs", "assets/shaders/default.fs");

    // texture
    rockColorTexture = std::make_unique<ew::Texture>("assets/textures/rock_color.jpg");
}

Scene::~Scene()
{
}

void Scene::Update(float dt)
{
    batteries::Scene::Update(dt);

    /* body */
}

auto matrix = glm::mat4(1.0f);

void Scene::Render(void)
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

    // draw suzanne
    suzanne->draw();
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

    /* build debug ui here */

    ImGui::End();
}