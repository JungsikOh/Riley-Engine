#pragma once
#include "../Utilities/Delegate.h"

namespace Riley
{
  class Layer
  {
  public:
    Layer(const std::string& name = "Layer");
    virtual ~Layer() = default;

    virtual void OnAttach() {}
    virtual void OnDetach() {}
    virtual void OnUpdate() {}
    virtual void OnImGuiRender() {}

    const std::string& GetName() const { return m_debugName; }

  protected:
    std::string m_debugName;
  };
} // namespace Riley