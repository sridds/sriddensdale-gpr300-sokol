#pragma once

// batteries
#include "batteries/scene.h"
#include "batteries/opengl.h"

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

    // post processing effects
    std::unique_ptr<ew::Shader> fullscreen;
    std::unique_ptr<ew::Shader> chromaticAbberation;
    std::unique_ptr<ew::Shader> lensDistortion;
    std::unique_ptr<ew::Shader> grayscale;
    std::unique_ptr<ew::Shader> invert;
    std::unique_ptr<ew::Shader> scanlines;
    std::unique_ptr<ew::Shader> vignette;
    std::unique_ptr<ew::Shader> filmGrain;
    std::unique_ptr<ew::Shader> sharpen;
    std::unique_ptr<ew::Shader> gaussianBlur;
    std::unique_ptr<ew::Shader> boxBlur;

    float ambient = 1.0f;
    float diffuse = 1.0f;
    float specular = 1.0f;
    float shininess = 1.0f;
    float lightPos[3] = { 2.0f, 2.0f, 2.0f };

    GLuint fbo;
    GLuint fboTexture;
    GLuint fboDepth;
};
