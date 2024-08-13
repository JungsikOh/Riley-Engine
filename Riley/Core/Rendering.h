#pragma once

// DirectX Helper
#include "../Utilities/Timer.h"
#include <Windows.h>
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <stdexcept>

namespace Riley
{

static RileyTimer timer;
static RileyTimer AppTimer;

#define SAFE_RELEASE(x)                                                                                                               \
    if (x)                                                                                                                            \
    {                                                                                                                                 \
        x->Release();                                                                                                                 \
        x = nullptr;                                                                                                                  \
    }

#define SAFE_DELETE(x)                                                                                                                \
    if (x)                                                                                                                            \
    {                                                                                                                                 \
        delete x;                                                                                                                     \
        x = nullptr;                                                                                                                  \
    }

#define SAFE_DELETE_ARRAY(x)                                                                                                          \
    if (x)                                                                                                                            \
    {                                                                                                                                 \
        delete[] x;                                                                                                                   \
        x = nullptr;                                                                                                                  \
    }

#define SAFE_CREATE(_type, _name)                                                                                                     \
    _type _name{};                                                                                                                    \
    memset(&_name, 0, sizeof(_type));

inline bool HR(HRESULT hr)
{
    if (FAILED(hr))
    {
        throw std::exception();
    }
    return true;
}

} // namespace Riley