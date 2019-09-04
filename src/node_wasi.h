#ifndef SRC_NODE_WASI_H_
#define SRC_NODE_WASI_H_

#if defined(NODE_WANT_INTERNALS) && NODE_WANT_INTERNALS

#include "base_object-inl.h"
#include "uvwasi.h"

namespace node {
namespace wasi {


class WASI : public BaseObject {
 public:
  WASI(Environment* env,
       v8::Local<v8::Object> object,
       v8::Local<v8::Value> memory,
       uvwasi_options_t* options);
  static void New(const v8::FunctionCallbackInfo<v8::Value>& args);
  SET_NO_MEMORY_INFO() 
  SET_MEMORY_INFO_NAME(WASI)
  SET_SELF_SIZE(WASI)

  static void ArgsGet(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void ArgsSizesGet(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void EnvironGet(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void EnvironSizesGet(const v8::FunctionCallbackInfo<v8::Value>& args);

 private:
  ~WASI() override;
  inline uvwasi_errno_t writeUInt32(uint32_t value, uint32_t offset);
  uvwasi_t uvw_;
  v8::Persistent<v8::ArrayBufferView> memory_;
};


} // namespace wasi
} // namespace node

#endif  // defined(NODE_WANT_INTERNALS) && NODE_WANT_INTERNALS

#endif  // SRC_NODE_WASI_H_