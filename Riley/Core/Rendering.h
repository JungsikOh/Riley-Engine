#pragma once

// DirectX Helper
#include "../Utilities/Timer.h"
#include <Windows.h>
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <stdexcept>

namespace Riley {

static RileyTimer timer; 
static RileyTimer AppTimer;

#define SAFE_RELEASE(x)                                                        \
    if (x) {                                                                   \
        x->Release();                                                          \
        x = nullptr;                                                           \
    }

#define SAFE_DELETE(x)                                                         \
    if (x) {                                                                   \
        delete x;                                                              \
        x = nullptr;                                                           \
    }

#define SAFE_DELETE_ARRAY(x)                                                   \
    if (x) {                                                                   \
        delete[] x;                                                            \
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