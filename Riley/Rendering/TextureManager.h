#pragma once
#include "../Graphics/DXResource.h"
#include "../Utilities/Singleton.h"

namespace Riley
{
using TextureHandle = uint64;
inline constexpr TextureHandle const INVALID_TEXTURE_HANDLE = uint64(-1);

class TextureManager : public Singleton<TextureManager>
{
    friend class Singleton<TextureManager>;

  public:
    void Initialize(ID3D11Device* device, ID3D11DeviceContext* context);
    void Destroy();

    TextureHandle LoadTexture(std::wstring const& name);
    TextureHandle LoadTexture(std::string const& name);

    DXResource* GetTextureView(TextureHandle texHandle) const;
    void SetMipMaps(bool mipmaps);

  private:
    ID3D11Device* m_device;
    ID3D11DeviceContext* m_context;
    bool mipmaps = true;
    TextureHandle handle = INVALID_TEXTURE_HANDLE;
    std::unordered_map<TextureHandle, std::unique_ptr<DXResource>> textureMap{};
    std::unordered_map<std::wstring, TextureHandle> loadedTextures{};

  private:
    TextureManager() = default;
    TextureManager(TextureManager const&) = delete;
    TextureManager& operator=(TextureManager const&) = delete;
    ~TextureManager() = default;

    TextureHandle LoadWICTexture(std::wstring const& name);
};
#define g_TextureManager TextureManager::Get()
} // namespace Riley