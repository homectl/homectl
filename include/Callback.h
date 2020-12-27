#pragma once

template <typename Sig>
class Callback;

template <typename R, typename... Args>
class Callback<R(Args...)> {
  using Function = R(void *, Args...);
  void *object_ = nullptr;
  Function *method_ = nullptr;

  template <typename Object, R (Object::*Method)(Args...)>
  static R invoke(void *self, Args... args) {
    return (static_cast<Object *>(self)->*Method)(args...);
  }

 public:
  R operator()(Args... args) const {
    if (method_) {
      return method_(object_, args...);
    }
    return R();
  }

  template <typename Object, R (Object::*Method)(Args...)>
  void listen(Object *self) {
    object_ = self;
    method_ = invoke<Object, Method>;
  }
};

#define connect(EVENT, CLASS, METHOD, OBJECT) \
  EVENT.listen<CLASS, &CLASS::METHOD>(&OBJECT)
