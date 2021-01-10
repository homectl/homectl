#pragma once

#include <utility>

template <typename Registry>
struct EventRegistered {
  typename Registry::callback_type &callback;
  typename Registry::object_type &object;

  void listen() { Registry::listen(callback, object); }
};

template <typename Sig>
class Callback;

template <typename R, typename... Args>
class Callback<R(Args...)> {
  using Function = R(void *, Args...);
  void *object_;
  Function *method_;

  template <typename Object, R (Object::*Method)(Args...)>
  static R invoke(void *self, Args... args) {
    return (static_cast<Object *>(self)->*Method)(args...);
  }

  template <typename Object, R (Object::*Method)(Args...) const>
  static R invoke(void *self, Args... args) {
    return (static_cast<Object const *>(self)->*Method)(args...);
  }

  template <typename Object, R (Object::*Method)(Args...)>
  struct DeferredListen {
    using callback_type = Callback<R(Args...)>;
    using object_type = Object;

    static void listen(callback_type &callback, object_type &object) {
      callback.template listen<Object, Method>(&object);
    }
  };

  template <typename Object, R (Object::*Method)(Args...)>
  void listen(Object *self) {
    object_ = self;
    method_ = invoke<Object, Method>;
  }

  template <typename Object, R (Object::*Method)(Args...) const>
  void listen(Object *self) {
    object_ = self;
    method_ = invoke<Object, Method>;
  }

 public:
  Callback() : object_(nullptr), method_(nullptr) {}

  R operator()(Args... args) const {
    if (method_) {
      return method_(object_, args...);
    }
    return R();
  }

  template <typename Object, R (Object::*Method)(Args...)>
  EventRegistered<DeferredListen<Object, Method>> listen(Object &object) {
    return {*this, object};
  }
};

#define EV_OBJECT(CLASS)                                \
 public:                                                \
  template <typename Registry, typename... Rest>        \
  CLASS(EventRegistered<Registry> evt, Rest &&... rest) \
      : CLASS(std::forward<Rest>(rest)...) {            \
    evt.listen();                                       \
  };                                                    \
                                                        \
  void loop();                                          \
                                                        \
 private:
