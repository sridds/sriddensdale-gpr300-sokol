#pragma once

// batteries
#include "batteries/scene.h"
#include "batteries/lights.h"

// ew
#include "ew/model.h"
#include "ew/mesh.h"
#include "ew/shader.h"
#include "ew/texture.h"
#include "ew/procGen.h"

// glm
#include "glm/glm.hpp"

class Scene final : public batteries::Scene
{
  public:
    Scene();
    virtual ~Scene();

    void Update(float dt);
    void Render(void);
    void Debug(void);

  private:
    void renderScene(ew::Shader &shader);
    void initializeInstanceData(void);
    void initPlane();

    static const unsigned int SCREEN_WIDTH = 800, SCREEN_HEIGHT = 600;
    static const int WIDTH = 3;

    std::unique_ptr<ew::Model> suzanne;
    std::unique_ptr<ew::Shader> geometry;
    std::unique_ptr<ew::Shader> deferred;
    std::unique_ptr<ew::Shader> lightsphere;
    std::unique_ptr<ew::Texture> texture;
    ew::Mesh sphere;
    
    std::vector<glm::mat4> model_instances;
    std::vector<batteries::light_t> light_instances;
};