#pragma once

#include <functional>
#include <iostream>
#include <vector>

/// <summary>
/// delegate pattern은 객체 합성이 상속과 동일하게 코드 재사용을 할 수 있도록 하는 객체 지향 디자인 패턴이다.
/// 객체 합성이란, 클래스 상속에 대한 대안으로 다른 객체를 여러 개 붙여서 새로운 기능 혹은 객체를 구성하는 것이다.
/// 객체를 합성하기 위해서는 합성에 들어가는 객체들의 인터페이스를 명확하게 정의해 두어야 한다.
/// </summary>

/// <summary>
/// delegation이란, 두 객체가 하나의 요청을 처리한다. 수신 객체가 연산의 처리를 위임자(delegate)에게 보낸다.
/// 이는 자식클래스가 부모 클래스에게 요청을 전달하는 것과 유사한 방법이다.
/// 즉, 객체가 자신의 기능을 다른 객체에게 위임하여 기능을 실행하는 디자인 패턴.
/// </summary>

typedef void (*Actionf)(int);
typedef void (*Actioni)(int);

typedef void (*Changed2i)(const int&, const int&);
typedef void (*Changed2f)(const float&, const float&);
typedef void (*Changed3f)(const float&, const float&, const float&);

typedef void (*CallbackFloatPtr)(const float*);

typedef void (*Action)();

// https://github.com/SamCZ/Aurora/blob/main/src/Aurora/Core/Delegate.hpp
// https://discord.com/channels/786048655596847106/786052698994769930/936352549886369852

// 객체의 메서드 포인터 타입을 정의
template <class UserClass, typename ReturnType, typename... ArgsTypes>
struct MethodAction {
    typedef ReturnType (UserClass::*Type)(ArgsTypes...);
};

// 함수 포인터 타입을 정의
template <typename ReturnType, typename... ArgsTypes> struct FunctionAction {
    typedef ReturnType (*Type)(ArgsTypes...);
};

template <typename ReturnType = void, typename... ArgsTypes> class IDelegate {
  public:
    virtual ReturnType Invoke(ArgsTypes&&... args) = 0;
    virtual ReturnType Invoke(ArgsTypes&... args) = 0;
};

// 특정 객체의 멤버 메서드를 호출하기 위한 delegate class
template <class UserClass, typename ReturnType, typename... ArgsTypes>
class MethodDelegate : public IDelegate<ReturnType, ArgsTypes...> {
  public:
    typedef typename MethodAction<UserClass, ReturnType, ArgsTypes...>::Type
        MethodPtr;

  private:
    UserClass* m_Instance;
    MethodPtr m_Method;

  public:
    inline MethodDelegate(UserClass* instance, MethodPtr method)
        : m_Instance(instance), m_Method(method) {}

    ReturnType Invoke(ArgsTypes&&... args) override {
        return std::invoke(m_Method, *m_Instance, args...);
    }
};

// 정적 메서드를 호출하기 위한 delegate class
template <typename ReturnType, typename... ArgsTypes>
class StaticMethodDel : public IDelegate<ReturnType, ArgsTypes...> {
  public:
    typedef typename FunctionAction<ReturnType, ArgsTypes...>::Type FunctionPtr;

  private:
    FunctionPtr m_Method;

  public:
    StaticMethodDel() {}
    inline StaticMethodDel(FunctionPtr method) : m_Method(method) {}

    ReturnType Invoke(ArgsTypes&&... args) override {
        return std::invoke(m_Method, args...);
    }
    ReturnType Invoke(ArgsTypes&... args) override {
        return std::invoke(m_Method, args...);
    }
};

// 함수를 호출하기 위한 delegate class
template <typename ReturnType, typename... ArgsTypes>
class FunctionDelegate : public IDelegate<ReturnType, ArgsTypes...> {
  public:
    typedef typename FunctionAction<ReturnType, ArgsTypes...>::Type FunctionPtr;

  private:
    FunctionPtr m_Function;

  public:
    inline explicit FunctionDelegate(FunctionPtr function)
        : m_Function(function) {}

    ReturnType Invoke(ArgsTypes&&... args) override {
        return std::invoke(m_Function, args...);
    }

    ReturnType Invoke(ArgsTypes&... args) override {
        return std::invoke(m_Function, args...);
    }
};

#if INTPTR_MAX == INT64_MAX
typedef uint64_t EventID;
#else
typedef uint32_t EventID;
#endif

// delegate를 관리하고 호출하는 이벤트 시스템
template <typename... ArgsTypes> class EventEmitter {
  public:
    typedef IDelegate<void, ArgsTypes...>* Delegate;

  private:
    std::vector<Delegate> m_Delegates;

  public:
    ~EventEmitter() {
        for (Delegate delegate : m_Delegates) {
            delete delegate;
        }
    }

    EventID Bind(Delegate delegate) {
        m_Delegates.push_back(delegate);
        return reinterpret_cast<EventID>(delegate);
    }

    EventID Bind(typename FunctionAction<void, ArgsTypes...>::Type function) {
        return Bind(new FunctionDelegate<void, ArgsTypes...>(function));
    }

    template <class UserClass>
    EventID
    Bind(UserClass* instance,
         typename MethodAction<UserClass, void, ArgsTypes...>::Type method) {
        return Bind(new MethodDelegate<UserClass, void, ArgsTypes...>(instance,
                                                                      method));
    }

    bool Unbind(EventID eventId) {
        auto it = std::find(m_Delegates.begin(), m_Delegates.end(),
                            reinterpret_cast<Delegate>(eventId));

        if (it == m_Delegates.end())
            return false;

        delete *it;
        m_Delegates.erase(it);

        return true;
    }

    void Clear() { m_Delegates.clear(); }

    void operator()(ArgsTypes&... args) { Invoke(args...); }

    void Invoke(ArgsTypes&&... args) {
        for (Delegate delegate : m_Delegates) {
            delegate->Invoke(std::forward<ArgsTypes>(args)...);
        }
    }

    void Invoke(ArgsTypes&... args) {
        for (Delegate delegate : m_Delegates) {
            delegate->Invoke(args...);
        }
    }
};

class Event {
  private:
    std::vector<Action> actions;

  public:
    void Add(const Action& act) { actions.push_back(act); }
    void Invoke() const {
        for (Action act : actions)
            act();
    }
    void operator()() { Invoke(); }
    void Clear() { actions.clear(); }
};