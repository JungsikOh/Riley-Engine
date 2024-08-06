#include "TextureManager.h"
#include "../Core/Rendering.h"
#include "../Utilities/FileUtil.h"
#include "../Utilities/StringUtil.h"
#include "directxtk/WICTextureLoader.h"

namespace Riley
{
using namespace DirectX;

namespace
{
enum class TextureFormat
{
    DDS,
    BMP,
    JPG,
    PNG,
    TIFF,
    GIF,
    ICO,
    TGA,
    HDR,
    PIC,
    NotSupported
};

TextureFormat GetTextureFormat(std::string const& path)
{
    std::string extension = GetExtension(path);
    std::transform(extension.begin(), extension.end(), extension.begin(), [](unsigned char c) { return std::tolower(c); });

    if (extension == ".dds")
        return TextureFormat::DDS;
    else if (extension == ".bmp")
        return TextureFormat::BMP;
    else if (extension == ".jpg" || extension == ".jpeg")
        return TextureFormat::JPG;
    else if (extension == ".png")
        return TextureFormat::PNG;
    else if (extension == ".tiff" || extension == ".tif")
        return TextureFormat::TIFF;
    else if (extension == ".gif")
        return TextureFormat::GIF;
    else if (extension == ".ico")
        return TextureFormat::ICO;
    else if (extension == ".tga")
        return TextureFormat::TGA;
    else if (extension == ".hdr")
        return TextureFormat::HDR;
    else if (extension == ".pic")
        return TextureFormat::PIC;
    else
        return TextureFormat::NotSupported;
}

TextureFormat GetTextureFormat(std::wstring const& path)
{
    return GetTextureFormat(ToString(path));
}

constexpr uint32 MipmapLevels(uint32 width, uint32 height)
{
    uint32 levels = 1U;
    while ((width | height) >> levels)
        ++levels;
    return levels;
}
} // namespace

void TextureManager::Initialize(ID3D11Device* device, ID3D11DeviceContext* context)
{
    m_device = device;
    m_context = context;
    mipmaps = true;
}

void TextureManager::Destroy()
{
    m_device = nullptr;
    m_context = nullptr;
}

TextureHandle TextureManager::LoadTexture(std::wstring const& name)
{
    TextureFormat format = GetTextureFormat(name);

    switch (format)
    {
    case TextureFormat::BMP:
    case TextureFormat::PNG:
    case TextureFormat::JPG:
    case TextureFormat::TIFF:
    case TextureFormat::GIF:
    case TextureFormat::ICO:
        return LoadWICTexture(name);
    default:
        assert(false && "Unsupported Texture Format");
    }

    return INVALID_TEXTURE_HANDLE;
}

TextureHandle TextureManager::LoadWICTexture(std::wstring const& name)
{
    if (auto iter = loadedTextures.find(name); iter == loadedTextures.end())
    {
        ++handle;

        std::unique_ptr<ID3D11ShaderResourceView> viewPtr = nullptr;

        ID3D11ShaderResourceView* tempViewPtr = nullptr;
        ID3D11Texture2D* texPtr = nullptr;
        if (mipmaps)
        {
            HR(CreateWICTextureFromFile(m_device, m_context, name.c_str(), reinterpret_cast<ID3D11Resource**>(&texPtr), &tempViewPtr));
        }
        else
        {
            HR(CreateWICTextureFromFile(m_device, name.c_str(), reinterpret_cast<ID3D11Resource**>(&texPtr), &tempViewPtr));
        }
        viewPtr.reset(tempViewPtr);

        loadedTextures.insert({name, handle});
        textureMap.insert({handle, viewPtr.get()});
        texPtr->Release();

        return handle;
    }
    else
        return iter->second;
}

} // namespace Riley