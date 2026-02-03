#pragma once

// batteries
#include "batteries/scene.h"

// ew
#include "ew/model.h"
#include "ew/shader.h"
#include "ew/texture.h"

class Scene final : public batteries::Scene
{
  public:
    Scene();
    virtual ~Scene();

    void Update(float dt);
    void Render(void);
    void Debug(void);

  private:
    std::unique_ptr<ew::Model> suzanne;
    std::unique_ptr<ew::Shader> blinnphong;
    std::unique_ptr<ew::Texture> rockColorTexture;

    float ambient;
    float diffuse;
    float specular;
    float shininess;
    float lightPos[3] = { 0.0f, 0.0f, 0.0f };
};
