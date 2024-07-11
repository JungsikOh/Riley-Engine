#pragma once

// DirectX Helper
#include <Windows.h>
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <stdexcept>
#include "spdlog\spdlog.h"

namespace Riley {


#define SAFE_RELEASE(x)                                                          \
    if (x) {                                                                   \
        x->Release();                                                          \
        x = nullptr;                                                           \
    }

#define SAFE_CREATE(_type, _name)                                              \
    _type _name{};                                                             \
    memset(&_name, 0, sizeof(_type));

inline bool HR(HRESULT hr) {
    if (FAILED(hr)) {
        throw std::exception();
    }
    return true;
}

// const char* to wchar_t*
static const wchar_t* GetWC(const char* c) {
    const size_t cSize = strlen(c) + 1;
    wchar_t* wc = new wchar_t[cSize];
    mbstowcs(wc, c, cSize);
    return wc;
}

} // namespace Riley