#include "env-inl.h"
#include "util-inl.h"
#include "node.h"
#include "uv.h"
#include "uvwasi.h"
#include "node_wasi.h"

namespace node {
namespace wasi {

using v8::Array;
using v8::ArrayBufferView;
using v8::Context;
using v8::FunctionCallbackInfo;
using v8::FunctionTemplate;
using v8::Isolate;
using v8::Local;
using v8::Number;
using v8::Object;
using v8::String;
using v8::Uint32;
using v8::Value;


WASI::WASI(Environment* env,
           Local<Object> object,
           Local<Value> memory,
           uvwasi_options_t* options) : BaseObject(env, object) {
  /* uvwasi_errno_t err = */ uvwasi_init(&uvw_, options);

  if (memory->IsNull())
    memory_.Reset();
  else
    memory_.Reset(env->isolate(), memory.As<ArrayBufferView>());
}


WASI::~WASI() {
  /* TODO(cjihrig): Free memory. */
}


void WASI::New(const FunctionCallbackInfo<Value>& args) {
  CHECK(args.IsConstructCall());
  CHECK_EQ(args.Length(), 4);
  CHECK(args[0]->IsArray());
  CHECK(args[1]->IsArray());
  // CHECK(args[2]->IsArray());
  CHECK(args[3]->IsArrayBufferView() || args[3]->IsNull());

  Environment* env = Environment::GetCurrent(args);
  Local<Context> context = env->context();
  Local<Array> argv = args[0].As<Array>();
  const uint32_t argc = argv->Length();
  uvwasi_options_t options;

  options.fd_table_size = 3;
  options.argc = argc;
  options.argv = argc == 0 ? nullptr : new char*[argc];

  for (uint32_t i = 0; i < argc; i++) {
    auto arg = argv->Get(context, i).ToLocalChecked();
    CHECK(arg->IsString());
    node::Utf8Value str(env->isolate(), arg);
    options.argv[i] = strdup(*str);
    CHECK_NOT_NULL(options.argv[i]);
  }

  Local<Array> env_pairs = args[1].As<Array>();
  const uint32_t envc = env_pairs->Length();
  options.envp = new char*[envc + 1];
  for (uint32_t i = 0; i < envc; i++) {
    auto pair = env_pairs->Get(context, i).ToLocalChecked();
    CHECK(pair->IsString());
    node::Utf8Value str(env->isolate(), pair);
    options.envp[i] = strdup(*str);
    CHECK_NOT_NULL(options.envp[i]);
  }
  options.envp[envc] = nullptr;

  // TODO(cjihrig): Process the preopens for real.
  options.preopenc = 1;
  options.preopens = (uvwasi_preopen_t*) calloc(1, sizeof(uvwasi_preopen_t));
  options.preopens[0].mapped_path = "/var";
  options.preopens[0].real_path = ".";

  new WASI(env, args.This(), args[3], &options);

  if (options.argv != nullptr) {
    for (uint32_t i = 0; i < argc; i++)
      free(options.argv[i]);
    delete[] options.argv;
  }

  if (options.envp != nullptr) {
    for (uint32_t i = 0; options.envp[i]; i++)
      free(options.envp[i]);
    delete[] options.envp;
  }
}


void WASI::ArgsGet(const FunctionCallbackInfo<Value>& args) {
  WASI* wasi;
  ASSIGN_OR_RETURN_UNWRAP(&wasi, args.This());

  char** args_get_argv = (char**) calloc(3, sizeof(char*));
  char buf[4096];
  uvwasi_errno_t err = uvwasi_args_get(&wasi->uvw_, args_get_argv, buf);

  printf("err = %d\n", err);
  printf("args[0] = %s\n", args_get_argv[0]);
  printf("args[1] = %s\n", args_get_argv[1]);
  printf("args[2] = %s\n", args_get_argv[2]);
  args.GetReturnValue().Set(err);
}


void WASI::ArgsSizesGet(const FunctionCallbackInfo<Value>& args) {
  WASI* wasi;
  size_t argc;
  size_t argv_buf_size;
  CHECK_EQ(args.Length(), 2);
  CHECK(args[0]->IsUint32());
  CHECK(args[1]->IsUint32());
  ASSIGN_OR_RETURN_UNWRAP(&wasi, args.This());
  uvwasi_errno_t err = uvwasi_args_sizes_get(&wasi->uvw_,
                                             &argc,
                                             &argv_buf_size);
  if (err == UVWASI_ESUCCESS)
    err = wasi->writeUInt32(argc, args[0].As<Uint32>()->Value());

  if (err == UVWASI_ESUCCESS)
    err = wasi->writeUInt32(argv_buf_size, args[1].As<Uint32>()->Value());

  args.GetReturnValue().Set(err);
}


void WASI::EnvironGet(const FunctionCallbackInfo<Value>& args) {
  WASI* wasi;
  CHECK_EQ(args.Length(), 2);
  CHECK(args[0]->IsUint32());
  CHECK(args[1]->IsUint32());
  ASSIGN_OR_RETURN_UNWRAP(&wasi, args.This());

  Environment* env = wasi->env();
  Local<ArrayBufferView> abv = PersistentToLocal::Default(env->isolate(),
                                                          wasi->memory_);
  // if (!abv->HasBuffer())
  //   return UVWASI_ENOBUFS;
  CHECK(abv->HasBuffer());

  uint8_t* buf = static_cast<uint8_t*>(abv->Buffer()->GetContents().Data());
  char** environ = (char**) &buf[args[0].As<Uint32>()->Value()];
  char* environ_buf = (char*) &buf[args[1].As<Uint32>()->Value()];
  uvwasi_errno_t err = uvwasi_environ_get(&wasi->uvw_, environ, environ_buf);
  printf("err = %d\n", err);
  for (int i = 0; i < 34; i++)
    printf("environ[%d] = %s\n", i, environ[i]);
  args.GetReturnValue().Set(err);
}


void WASI::EnvironSizesGet(const FunctionCallbackInfo<Value>& args) {
  WASI* wasi;
  size_t envc;
  size_t env_buf_size;
  CHECK_EQ(args.Length(), 2);
  CHECK(args[0]->IsUint32());
  CHECK(args[1]->IsUint32());
  ASSIGN_OR_RETURN_UNWRAP(&wasi, args.This());
  uvwasi_errno_t err = uvwasi_environ_sizes_get(&wasi->uvw_,
                                                &envc,
                                                &env_buf_size);
  if (err == UVWASI_ESUCCESS)
    err = wasi->writeUInt32(envc, args[0].As<Uint32>()->Value());

  if (err == UVWASI_ESUCCESS)
    err = wasi->writeUInt32(env_buf_size, args[1].As<Uint32>()->Value());

  args.GetReturnValue().Set(err);
}


inline uvwasi_errno_t WASI::writeUInt32(uint32_t value, uint32_t offset) {
  Environment* env = this->env();
  Local<ArrayBufferView> abv = PersistentToLocal::Default(env->isolate(),
                                                          this->memory_);
  if (!abv->HasBuffer())
    return UVWASI_ENOBUFS;

  uint8_t* buf = static_cast<uint8_t*>(abv->Buffer()->GetContents().Data());
  // Bounds check. UVWASI_EOVERFLOW

  buf[offset++] = value & 0xFF;
  buf[offset++] = (value >> 8) & 0xFF;
  buf[offset++] = (value >> 16) & 0xFF;
  buf[offset] = (value >> 24) & 0xFF;
  return 0;
}


static void Initialize(Local<Object> target,
                       Local<Value> unused,
                       Local<Context> context,
                       void* priv) {
  Environment* env = Environment::GetCurrent(context);

  Local<FunctionTemplate> tmpl = env->NewFunctionTemplate(WASI::New);
  auto wasi_wrap_string = FIXED_ONE_BYTE_STRING(env->isolate(), "WASI");
  tmpl->InstanceTemplate()->SetInternalFieldCount(1);
  tmpl->SetClassName(wasi_wrap_string);

  env->SetProtoMethod(tmpl, "args_get", WASI::ArgsGet);
  env->SetProtoMethod(tmpl, "args_sizes_get", WASI::ArgsSizesGet);
  env->SetProtoMethod(tmpl, "environ_get", WASI::EnvironGet);
  env->SetProtoMethod(tmpl, "environ_sizes_get", WASI::EnvironSizesGet);

  target->Set(env->context(),
              wasi_wrap_string,
              tmpl->GetFunction(context).ToLocalChecked()).ToChecked();
}


} // namespace wasi
} // namespace node

NODE_MODULE_CONTEXT_AWARE_INTERNAL(wasi, node::wasi::Initialize)
