#include "scene.h"

// imgui
#include "imgui/imgui.h"
#include "imguizmo/imguizmo.h"

// glm
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"

// batteries
#include "batteries/opengl.h"

ew::Texture* rockColorTexture;
ew::Texture* gradient;

Scene::Scene()
{
    suzanne = std::make_unique<ew::Model>("assets/models/suzanne.obj");
    toon = std::make_unique<ew::Shader>("assets/shaders/default.vs", "assets/shaders/windwaker.fs");
    
    // texture
    ew::Texture rockColorTexture = ew::Texture("assets/textures/rock_color.jpg");
    ew::Texture gradient = ew::Texture("assets/textures/ZAtoon.png");
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
    glBindTextureUnit(0, rockColorTexture->getID()); 

    glActiveTexture(GL_TEXTURE1);
    glBindTextureUnit(1, gradient->getID()); 

    toon->use();
    toon->setInt("texture0", 0);
    toon->setInt("zatoon", 1);

    // scene matrices
    toon->setMat4("model", matrix);
    toon->setMat4("view_proj", view_proj);
    toon->setVec3("camera_position", camera.position);
    toon->setInt("_MainTex", 0);

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

    /* build debug ui here */

    ImGui::End();
}