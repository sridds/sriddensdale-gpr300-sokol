#include "scene.h"

// batteries
#include "batteries/math.h"
#include "batteries/opengl.h"
#include "batteries/materials.h"

// ew
#include "ew/procGen.h"
#include <iostream>

// imgui
#include "imgui/imgui.h"
#include "imguizmo/imguizmo.h"

constexpr int kFramebufferWidth = 800;
constexpr int kFramebufferHeight = 600;
constexpr float orbit_radius = 2.0f;

struct FullscreenQuad
{
    GLuint vao;
    GLuint vbo;

    void Initialize()
    {
        // clang-format off
        float quad_vertices[] = {
            // pos (x, y) texcoord (u, v)
            -1.0f,  1.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f,
             1.0f, -1.0f, 1.0f, 0.0f,

            -1.0f,  1.0f, 0.0f, 1.0f,
             1.0f, -1.0f, 1.0f, 0.0f,
             1.0f,  1.0f, 1.0f, 1.0f,
        };
        // clang-format on

        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);

        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);

        glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), &quad_vertices, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(sizeof(float) * 2));

        glBindVertexArray(0);
    }
} fullscreen_quad;

struct Framebuffer
{
    GLuint fbo;
    GLuint position;
    GLuint depth;
    GLuint colorBuffers[3];

    void Initialize()
    {
        glGenFramebuffers(1, &fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);

        // 0 is world pos, 1 is world normal, 2 is albedo
        int formats[3] = { GL_RGB32F, GL_RGB16F, GL_RGB16F };

        for (size_t i = 0; i < 3; i++)
        {
            glGenTextures(1, &colorBuffers[i]);
            glBindTexture(GL_TEXTURE_2D, colorBuffers[i]);
            glTexStorage2D(GL_TEXTURE_2D, 1, formats[i], kFramebufferWidth, kFramebufferHeight);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

            glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, colorBuffers[i], 0);
        }

        const GLenum drawBuffers[3] = {
            GL_COLOR_ATTACHMENT0,
            GL_COLOR_ATTACHMENT1,
            GL_COLOR_ATTACHMENT2,
        };
        glDrawBuffers(3, drawBuffers);

        glGenRenderbuffers(1, &depth);
        glBindRenderbuffer(GL_RENDERBUFFER, depth);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, kFramebufferWidth, kFramebufferHeight);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            printf("Not so victorious\n");
        }

        glBindTexture(GL_TEXTURE_2D, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
} framebuffer;

struct LightVolumeBuffer
{
    GLuint fbo;
    GLuint color;
    GLuint depth;

    void Initialize()
    {
        glGenFramebuffers(1, &fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);

        glGenTextures(1, &color);
        glBindTexture(GL_TEXTURE_2D, color);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, kFramebufferWidth, kFramebufferHeight, 0, GL_RGBA, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color, 0);

        glGenTextures(1, &depth);
        glBindTexture(GL_TEXTURE_2D, depth);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, kFramebufferWidth, kFramebufferHeight, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, depth, 0);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            printf("Not so victorious\n");
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
} lightvolumebuffer;

struct Material
{
    float ambient = 0.1f;
    float diffuse = 0.5f;
    float specular = 0.5f;
    float shininess = 32.0f; 
} material;

struct
{
    float light_brightness = 1.0f;
    float light_radius = 5.0f; 
    bool draw_light_volume = false;
} debug;

Scene::Scene()
{
    // assets
    suzanne = std::make_unique<ew::Model>("assets/models/suzanne.obj");
    texture = std::make_unique<ew::Texture>("assets/textures/rock_color.jpg");
    sphere.load(ew::createSphere(1.0f, 8));

    // shaders
    geometry = std::make_unique<ew::Shader>("assets/shaders/geometry.vs",  "assets/shaders/geometry.fs");
    lightsphere = std::make_unique<ew::Shader>("assets/shaders/light.vs",     "assets/shaders/light.fs");
    deferred = std::make_unique<ew::Shader>("assets/shaders/default2.vs",  "assets/shaders/default2.fs");

    // init
    framebuffer.Initialize();
    lightvolumebuffer.Initialize();
    fullscreen_quad.Initialize();

    initializeInstanceData();
}

Scene::~Scene()
{
}

void Scene::initializeInstanceData(void)
{
    auto size = (WIDTH - (-WIDTH) + 1) * (WIDTH - (-WIDTH) + 1);
    model_instances.resize(size);
    light_instances.resize(size);

    std::cout << size << std::endl;

    auto i = 0;
    for (auto x = -WIDTH; x <= WIDTH; x++)
    {
        for (auto y = -WIDTH; y <= WIDTH; y++, i++)
        {
            const auto position = glm::vec3(x * 3.0f, 0, y * 3.0f);
            const auto orbit = batteries::random_point_on_sphere();

            light_instances[i] = {
                .color = batteries::random_color(),
                .position = glm::vec4(position, 1.0f) + orbit * orbit_radius,
            };

            model_instances[i] = batteries::random_model_matrix(position);
        }
    }
}

void Scene::Update(float dt)
{
    batteries::Scene::Update(dt);
}

void Scene::Render(void)
{
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_DEPTH_TEST);

    const auto view_proj = camera.Projection() * camera.View();

    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.fbo);
    glViewport(0, 0, kFramebufferWidth, kFramebufferHeight);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // geometry pass
    geometry->use();
    geometry->setMat4("view_proj", view_proj);
    geometry->setInt("_MainTex", 0);
    glBindTextureUnit(0, texture->getID());

    for (int i = 0; i < (int)model_instances.size(); i++)
    {
        geometry->setMat4("model", model_instances[i]);
        suzanne->draw();
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, kFramebufferWidth, kFramebufferHeight);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // handle lighting
    deferred->use();
    deferred->setVec3("camera_position", camera.position);
    deferred->setFloat("material.shininess", material.shininess);
    deferred->setVec3("material.diffuse",    glm::vec3(material.diffuse));
    deferred->setVec3("material.specular",   glm::vec3(material.specular));
    deferred->setVec3("material.ambient",    glm::vec3(material.ambient));
    deferred->setInt("gPositions", 0);
    deferred->setInt("gNormals",   1);
    deferred->setInt("gAlbedoSpec",    2);
    glBindTextureUnit(0, framebuffer.colorBuffers[0]);
    glBindTextureUnit(1, framebuffer.colorBuffers[1]);
    glBindTextureUnit(2, framebuffer.colorBuffers[2]);

    // Set shader uniform
    for (int i = 0; i < light_instances.size(); i++)
    {
        // Creates prefix "_PointLights[0]." etc
        std::string prefix = "_PointLights[" + std::to_string(i) + "].";
        deferred->setVec3(prefix + "position", light_instances[i].position);
        deferred->setFloat(prefix + "radius", debug.light_radius);
        deferred->setVec4(prefix + "color", glm::vec4(light_instances[i].color.r, light_instances[i].color.g,  light_instances[i].color.b, 1.0f) * debug.light_brightness);
    }

    glBindVertexArray(fullscreen_quad.vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer.fbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(
        0, 0, kFramebufferWidth, kFramebufferHeight,
        0, 0, kFramebufferWidth, kFramebufferHeight,
        GL_DEPTH_BUFFER_BIT, GL_NEAREST
    );
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // draw light volumes
    if (debug.draw_light_volume)
    {
        lightsphere->use();
        lightsphere->setMat4("view_proj", view_proj);

        for (int i = 0; i < (int)light_instances.size(); i++)
        {
            glm::mat4 m = glm::translate(glm::mat4(1.0f), glm::vec3(light_instances[i].position));
            m = glm::scale(m, glm::vec3(0.1f));
            lightsphere->setMat4("model", m);
            lightsphere->setVec3("color", glm::vec3(light_instances[i].color));
            sphere.draw();
        }
    }
}

void Scene::Debug(void)
{
    cameracontroller.Debug();

    ImGui::Begin("Controls", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

    if (ImGui::CollapsingHeader("Lights"))
    {
        ImGui::SliderFloat("Brightness", &debug.light_brightness, 0.1f, 10.0f);
        ImGui::SliderFloat("Radius", &debug.light_radius, 0.1f, 100.0f);
        ImGui::Checkbox("Draw Lights", &debug.draw_light_volume);
    }

    if (ImGui::CollapsingHeader("Material"))
    {
        ImGui::SliderFloat("Material Shininess", &material.shininess, 1.0f, 128.0f);
    }

    if (ImGui::CollapsingHeader("GBuffers"))
    {
        ImVec2 texSize = ImVec2(kFramebufferWidth / 4, kFramebufferHeight / 4);

        for (size_t i = 0; i < 3; i++)
        {
            ImGui::Image((ImTextureID)(intptr_t)framebuffer.colorBuffers[i], texSize, ImVec2(0, 1), ImVec2(1, 0));
        }
    }
    ImGui::End();
}