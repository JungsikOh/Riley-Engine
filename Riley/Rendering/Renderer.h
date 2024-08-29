#pragma once
#include "../Core/Window.h"
#include "../Graphics/DXBuffer.h"
#include "../Graphics/DXConstantBuffer.h"
#include "../Graphics/DXDepthStencilBuffer.h"
#include "../Graphics/DXRenderPass.h"
#include "../Graphics/DXRenderTarget.h"
#include "Camera.h"
#include "Components.h"
#include "ConstantBuffers.h"
#include "RenderSetting.h"
#include "SceneViewport.h"
#include "ShaderManager.h"
#include "TextureManager.h"

namespace Riley
{

class DXSampler;
class DXBlendState;
class DXRasterizerState;
class DXDepthStencilState;

class Engine;
class Camera;
class Input;

class Renderer
{
    friend class Engine;

  public:
    Renderer() = default;
    Renderer(entt::registry& reg, ID3D11Device* device, ID3D11DeviceContext* context, ID3DUserDefinedAnnotation* pAnnotation,
             Camera* camera, uint32 width, uint32 height);
    ~Renderer();

    void Tick(Camera*);
    void Update(float dt);

    void UpdateLights();


    void SetSceneViewport(const float& width = 0.0f, const float& height = 0.0f, const float& minDepth = 0.0f,
                          const float& maxDepth = 1.0f, const float& topLeftX = 0.0f, const float& topLeftY = 0.0f);

    void SetSceneViewport(SceneViewport const& viewport);
    DXRenderTarget* GetOffScreenRTV()
    {
        return postprocessPasses[!postprocessIndex].attachmentRTVs;
    }

    void Render(RenderSetting& _setting);

    void OnResize(uint32 width, uint32 height);
    void OnLeftMouseClicked(uint32 mx, uint32 my);
    entt::entity GetSelectedEntity() const
    {
        return selectedEntity;
    }
    void SetSelectedEntity(entt::entity e)
    {
        selectedEntity = e;
    }

  protected:
    uint32 m_width, m_height;

    /* Low Level APIs */
    ID3D11Device* m_device;
    ID3D11DeviceContext* m_context;
    ID3DUserDefinedAnnotation& m_annotation;

    /* App Level */
    entt::registry& m_reg;
    Camera* m_camera;
    SceneViewport m_currentSceneViewport;
    float m_currentDeltaTime;
    bool m_seleted = false;
    bool m_dragStartFlag = false;

    // Others
    ////////////////////////////
    RenderSetting renderSetting; // It's created in Editor Class
    entt::entity selectedEntity = entt::null;
    DirectX::BoundingBox lightBoundingBox;
    DirectX::BoundingFrustum lightBoundingFrustum;
    DirectX::BoundingFrustum lightBoundingFrustumCube[6];
    std::array<Vector4, SSAO_KERNEL_SIZE> ssaoKernel;

    // Resources
    DXBuffer* lights = nullptr;
    DXBuffer* outputTiledLights = nullptr;
    DXResource* uavTarget = nullptr;
    DXResource* debugTiledTexture = nullptr;
    DXResource* ssaoNoiseTex = nullptr;
    DXResource* blurTextureIntermediate = nullptr;
    TextureHandle sunTex = INVALID_TEXTURE_HANDLE;

    // cbuffers
    FrameBufferConsts frameBufferCPU;
    DXConstantBuffer<FrameBufferConsts>* frameBufferGPU = nullptr;
    ObjectConsts objectConstsCPU;
    DXConstantBuffer<ObjectConsts>* objectConstsGPU = nullptr;
    MaterialConsts materialConstsCPU;
    DXConstantBuffer<MaterialConsts>* materialConstsGPU = nullptr;
    PostprocessConsts postProcessCPU;
    DXConstantBuffer<PostprocessConsts>* postProcessGPU = nullptr;
    LightConsts lightConstsCPU;
    DXConstantBuffer<LightConsts>* lightConstsGPU = nullptr;
    ShadowConsts shadowConstsCPU;
    DXConstantBuffer<ShadowConsts>* shadowConstsGPU = nullptr;
    EntityIdConsts entityIdConstsCPU{};
    DXConstantBuffer<EntityIdConsts>* entityIdConstsGPU = nullptr;

    // Render States
    DXRasterizerState* solidRS;
    DXRasterizerState* wireframeRS;
    DXRasterizerState* cullNoneRS;
    DXRasterizerState* cullFrontRS;
    DXRasterizerState* depthBiasRS;

    DXDepthStencilState* solidDSS;
    DXDepthStencilState* noneDepthDSS;

    DXBlendState* opaqueBS;
    DXBlendState* additiveBS;
    DXBlendState* alphaBS;

    DXSampler* linearWrapSS;
    DXSampler* linearClampSS;
    DXSampler* linearBorderSS;
    DXSampler* pointWrapSS;
    DXSampler* pointClampSS;
    DXSampler* anisotropyWrapSS;
    DXSampler* shadowLinearBorderSS;

    // Render Target Views
    DXRenderTarget* hdrRTV;
    DXRenderTarget* gbufferRTV;
    DXRenderTarget* ambientLightingRTV;
    DXRenderTarget* ssaoRTV;
    DXRenderTarget* ssaoBlurRTV;
    DXRenderTarget* sunRTV;
    DXRenderTarget* pingPostprocessRTV;
    DXRenderTarget* pongPostprocessRTV;
    DXRenderTarget* entityIdRTV;

    // Depth Stencil Buffers(View)
    DXDepthStencilBuffer* hdrDSV;
    DXDepthStencilBuffer* gbufferDSV;
    DXDepthStencilBuffer* ambientLightingDSV;
    DXDepthStencilBuffer* ssaoDSV;
    DXDepthStencilBuffer* ssaoBlurDSV;
    DXDepthStencilBuffer* pingPostprocessDSV;
    DXDepthStencilBuffer* pongPostprocessDSV;
    DXDepthStencilBuffer* depthMapDSV;
    DXDepthStencilBuffer* shadowDepthMapDSV;
    DXDepthStencilBuffer* shadowCascadeMapDSV;
    DXDepthStencilBuffer* shadowDepthCubeMapDSV;
    DXDepthStencilBuffer* entityIdDSV;

    // Render Pass
    DXRenderPassDesc forwardPass;
    DXRenderPassDesc gbufferPass;
    DXRenderPassDesc deferredLightingPass;
    DXRenderPassDesc ssaoPass;
    DXRenderPassDesc shadowMapPass;
    DXRenderPassDesc shadowCascadeMapPass;
    DXRenderPassDesc shadowCubeMapPass;
    std::array<DXRenderPassDesc, 2> postprocessPasses;
    bool postprocessIndex = false;

  private:
    void CreateBuffers();
    void CreateSamplers();
    void CreateRenderStates();

    void CreateDepthStencilBuffers(uint32 width, uint32 height);
    void CreateRenderTargets(uint32 width, uint32 height);
    void CreateRenderPasses(uint32 width, uint32 height);

    void CreateGBuffer(uint32 width, uint32 height);
    void CreateOtherResources();

    void BindGlobals();
    void LightFrustumCulling(const Light& light);

    void PassForward();
    void PassForwardPhong();
    void PassGBuffer();
    void PassAmbient();
    void PassDeferredLighting();
    void PassTiledDeferredLighting();

    // Postprocessing
    void PassPostprocessing();
    void PassHalo();
    void PassGodsRay(const Light& light);
    void PassSSAO();
    void PassSSR();

    void PassShadowMapDirectional(const Light& light);
    void PassShadowMapCascade(const Light& light);
    void PassShadowMapSpot(const Light& light);
    void PassShadowMapPoint(const Light& light);

    void PassAABB();
    void PassLight();
    void PassEntityID();

    void DrawSun(entt::entity light);
    void BlurTexture(DXRenderTarget* src);
    void AddTexture(DXRenderTarget* dest, DXResource* src);
    void CopyTexture(DXResource* src);
};

} // namespace Riley