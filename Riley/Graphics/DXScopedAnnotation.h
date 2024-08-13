#pragma once

namespace Riley
{
class DXScopedAnnotation
{
  public:
    DXScopedAnnotation(ID3DUserDefinedAnnotation& annotation, char const* name);
    ~DXScopedAnnotation();

  private:
    ID3DUserDefinedAnnotation& m_annotation;
};
#define RILEY_SCOPED_ANNOTATION(annotation, name) DXScopedAnnotation RILEY_CONCAT(_annotation, __COUNTER__)(annotation, name)

} // namespace Riley