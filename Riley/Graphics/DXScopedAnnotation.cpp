#include "DXScopedAnnotation.h"
#include "../Utilities/StringUtil.h"

namespace Riley
{
DXScopedAnnotation::DXScopedAnnotation(ID3DUserDefinedAnnotation& annotation, char const* name) : m_annotation(annotation)
{
    m_annotation.BeginEvent(ToWideString(name).c_str());
}
DXScopedAnnotation::~DXScopedAnnotation()
{
    m_annotation.EndEvent();
}
} // namespace Riley