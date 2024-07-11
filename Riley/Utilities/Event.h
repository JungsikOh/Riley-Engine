#pragma once

#include <functional>
#include <iostream>
#include <vector>

/// <summary>
/// delegate pattern�� ��ü �ռ��� ��Ӱ� �����ϰ� �ڵ� ������ �� �� �ֵ��� �ϴ� ��ü ���� ������ �����̴�.
/// ��ü �ռ��̶�, Ŭ���� ��ӿ� ���� ������� �ٸ� ��ü�� ���� �� �ٿ��� ���ο� ��� Ȥ�� ��ü�� �����ϴ� ���̴�.
/// ��ü�� �ռ��ϱ� ���ؼ��� �ռ��� ���� ��ü���� �������̽��� ��Ȯ�ϰ� ������ �ξ�� �Ѵ�.
/// </summary>

/// <summary>
/// delegation�̶�, �� ��ü�� �ϳ��� ��û�� ó���Ѵ�. ���� ��ü�� ������ ó���� ������(delegate)���� ������.
/// �̴� �ڽ�Ŭ������ �θ� Ŭ�������� ��û�� �����ϴ� �Ͱ� ������ ����̴�.
/// ��, ��ü�� �ڽ��� ����� �ٸ� ��ü���� �����Ͽ� ����� �����ϴ� ������ ����.
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

// ��ü�� �޼��� ������ Ÿ���� ����
template <class UserClass, typename ReturnType, typename... ArgsTypes>
struct MethodAction {
    typedef ReturnType (UserClass::*Type)(ArgsTypes...);
};

// �Լ� ������ Ÿ���� ����
template <typename ReturnType, typename... ArgsTypes> struct FunctionAction {
    typedef ReturnType (*Type)(ArgsTypes...);
};

template <typename ReturnType = void, typename... ArgsTypes> class IDelegate {
  public:
    virtual ReturnType Invoke(ArgsTypes&&... args) = 0;
    virtual ReturnType Invoke(ArgsTypes&... args) = 0;
};

// Ư�� ��ü�� ��� �޼��带 ȣ���ϱ� ���� delegate class
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

// ���� �޼��带 ȣ���ϱ� ���� delegate class
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

// �Լ��� ȣ���ϱ� ���� delegate class
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

// delegate�� �����ϰ� ȣ���ϴ� �̺�Ʈ �ý���
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