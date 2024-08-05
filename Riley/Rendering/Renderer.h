#pragma once
#include "../Core/Window.h"
#include "../Graphics/DXBuffer.h"
#include "../Graphics/DXConstantBuffer.h"
#include "../Graphics/DXDepthStencilBuffer.h"
#include "../Graphics/DXRenderTarget.h"
#include "../Graphics/DXRenderPass.h"
#include "Camera.h"
#include "ConstantBuffers.h"
#include "Components.h"
#include "SceneViewport.h"
#include "ShaderManager.h"

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
   Renderer(entt::registry& reg, ID3D11Device* device, ID3D11DeviceContext* context, Camera* camera, uint32 width, uint32 height);
   ~Renderer();

   void Tick(Camera*);
   void Update(float dt);

   void SetSceneViewport(const float& width = 0.0f, const float& height = 0.0f, const float& minDepth = 0.0f,
                         const float& maxDepth = 1.0f, const float& topLeftX = 0.0f, const float& topLeftY = 0.0f);

   void SetSceneViewport(SceneViewport const& viewport);
   DXRenderTarget* GetOffScreenRTV()
   {
      return ambientLightingRTV;
   }

   void Render();

   void OnResize(uint32 width, uint32 height);
   void OnLeftMouseClicked(uint32 mx, uint32 my);

   private:
   uint32 m_width, m_height;

   /* Low Level APIs */
   ID3D11Device* m_device;
   ID3D11DeviceContext* m_context;

   /* App Level */
   entt::registry& m_reg;
   Camera* m_camera;
   SceneViewport m_currentSceneViewport;
   float m_currentDeltaTime;
   bool m_seleted = false;
   bool m_dragStartFlag = false;

   // Others
   ////////////////////////////
   DirectX::BoundingBox lightBoundingBox;

   // cbuffers
   FrameBufferConsts frameBufferCPU;
   DXConstantBuffer<FrameBufferConsts>* frameBufferGPU = nullptr;
   ObjectConsts objectConstsCPU;
   DXConstantBuffer<ObjectConsts>* objectConstsGPU = nullptr;
   MaterialConsts materialConstsCPU;
   DXConstantBuffer<MaterialConsts>* materialConstsGPU = nullptr;
   LightConsts lightConstsCPU;
   DXConstantBuffer<LightConsts>* lightConstsGPU = nullptr;
   ShadowConsts shadowConstsCPU;
   DXConstantBuffer<ShadowConsts>* shadowConstsGPU = nullptr;

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
   DXSampler* shadowLinearBorderSS;

   // Render Target Views
   DXRenderTarget* hdrRTV;
   DXRenderTarget* gbufferRTV;
   DXRenderTarget* ambientLightingRTV;
   DXRenderTarget* postProcessRTV;

   // Depth Stencil Buffers(View)
   DXDepthStencilBuffer* hdrDSV;
   DXDepthStencilBuffer* gbufferDSV;
   DXDepthStencilBuffer* ambientLightingDSV;
   DXDepthStencilBuffer* depthMapDSV;
   DXDepthStencilBuffer* shadowDepthMapDSV;
   DXDepthStencilBuffer* shadowDepthCubeMapDSV;

   // Render Pass
   DXRenderPassDesc forwardPass;
   DXRenderPassDesc gbufferPass;
   DXRenderPassDesc deferredLightingPass;
   DXRenderPassDesc shadowMapPass;
   DXRenderPassDesc shadowCubeMapPass;
   DXRenderPassDesc postProcessPass;

   private:
   void CreateBuffers();
   void CreateSamplers();
   void CreateRenderStates();

   void CreateDepthStencilBuffers(uint32 width, uint32 height);
   void CreateRenderTargets(uint32 width, uint32 height);
   void CreateRenderPasses(uint32 width, uint32 height);

   void CreateGBuffer(uint32 width, uint32 height);

   void BindGlobals();

   void PassForward();
   void PassSolid();
   void PassForwardPhong();
   void PassGBuffer();
   void PassAmbient();
   void PassDeferredLighting();

   void PassShadowMapDirectional(Light const& light);
   void PassShadowMapSpot(Light const& light);
   void PassShadowMapPoint(Light const& light);

   void PassAABB();
};

} // namespace Riley