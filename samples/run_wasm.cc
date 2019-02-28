// Copyright 2015 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "include/v8.h"
#include "include/libplatform/libplatform.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include<algorithm>

#include <map>
#include <string>

#include <iostream>
#include <fstream>
#include <iterator>
#include <vector>

using std::map;
using std::pair;
using std::string;

using v8::HandleScope;
using v8::Isolate;
using v8::Local;
using v8::MaybeLocal;
using v8::NewStringType;
using v8::String;
using v8::WasmCompiledModule;
using v8::WasmModuleObject;
using v8::Object;
using v8::Value;
using v8::Int32;
using v8::Function;
using args_type = Local<Value>[];

void ParseOptions(int argc,
                  char* argv[],
                  map<string, string>* options,
                  string* file) {
  for (int i = 1; i < argc; i++) {
    string arg = argv[i];
    size_t index = arg.find('=', 0);
    if (index == string::npos) {
      *file = arg;
    } else {
      string key = arg.substr(0, index);
      string value = arg.substr(index+1);
      (*options)[key] = value;
    }
  }
}

static std::vector<char> ReadAllBytes(char const* filename) {
  std::ifstream input(filename, std::ios::binary);

  std::vector<char> bytes(
        (std::istreambuf_iterator<char>(input)),
        (std::istreambuf_iterator<char>()));

  input.close();

  return bytes;
}

static std::vector<uint8_t> ReadAllUint8Bytes(char const* filename) {
  /*
  std::basic_ifstream<uint8_t> stream(filename, std::ios::in | std::ios::binary);    
  auto eos = std::istreambuf_iterator<uint8_t>();
  auto buffer = std::vector<uint8_t>(std::istreambuf_iterator<uint8_t>(stream), eos);

  return buffer;
  */
  std::ifstream input(filename, std::ios::binary);
  std::vector<uint8_t> data;

  std::for_each(
        std::istreambuf_iterator<char>(input),
        std::istreambuf_iterator<char>(),
        [&data](const char c) {
                  data.push_back(c);
              });

  input.close();

  return data;
}

int main(int argc, char* argv[]) {
  // Initialize V8.
  v8::V8::InitializeICUDefaultLocation(argv[0]);
  v8::V8::InitializeExternalStartupData(argv[0]);
  std::unique_ptr<v8::Platform> platform = v8::platform::NewDefaultPlatform();
  v8::V8::InitializePlatform(platform.get());
  v8::V8::Initialize();

  // Create a new Isolate and make it the current one.
  v8::Isolate::CreateParams create_params;
  create_params.array_buffer_allocator =
      v8::ArrayBuffer::Allocator::NewDefaultAllocator();
  v8::Isolate* isolate = v8::Isolate::New(create_params);
  {
    v8::Isolate::Scope isolate_scope(isolate);

    // Create a stack-allocated handle scope.
    v8::HandleScope handle_scope(isolate);

    // Create a new context.
    v8::Local<v8::Context> context = v8::Context::New(isolate);

    // Enter the context for compiling and running the hello world script.
    v8::Context::Scope context_scope(context);

    {
      string file = "samples/fib.wasm";
      std::vector<uint8_t> fileBytes = ReadAllUint8Bytes(file.c_str());

      fprintf(stderr, "Wasm lenght! '%i'.\n", (int)fileBytes.size());
      
      Local<WasmCompiledModule> module = WasmCompiledModule::DeserializeOrCompile(
        reinterpret_cast<v8::Isolate*>(isolate),
        WasmCompiledModule::BufferReference(0, 0),
        WasmCompiledModule::BufferReference(fileBytes.data(), fileBytes.size())
      ).ToLocalChecked();

      fprintf(stderr, "is Wasm module! '%i'.\n", module->IsWebAssemblyCompiledModule());

      args_type instance_args{module};

      Local<Object> module_instance_exports = context->Global()
          ->Get(context, String::NewFromUtf8(isolate, "WebAssembly"))
          .ToLocalChecked().As<Object>()
          ->Get(context, String::NewFromUtf8(isolate, "Instance"))
          .ToLocalChecked().As<Object>()
          ->CallAsConstructor(context, 1, instance_args)
          .ToLocalChecked().As<Object>()
          ->Get(context, String::NewFromUtf8(isolate, "exports"))
          .ToLocalChecked().As<Object>()
          ;
      
      args_type add_args{Int32::New(isolate, 5)};
      Local<Int32> adder_res = module_instance_exports
        ->Get(context, String::NewFromUtf8(isolate, "fib"))
        .ToLocalChecked().As<Function>()
        ->Call(context, context->Global(), 1, add_args)
        .ToLocalChecked().As<Int32>();

      printf("fib 5 = %d\n", adder_res->Value());
    }

    {
      map<string, string> options;
      string file;
      ParseOptions(argc, argv, &options, &file);
      if (file.empty()) {
        fprintf(stderr, "No script was specified.\n");
        return 1;
      } else {
        fprintf(stderr, "Wasm found!\n");
      }
      std::vector<uint8_t> fileBytes = ReadAllUint8Bytes(file.c_str());

      fprintf(stderr, "Wasm lenght! '%i'.\n", (int)fileBytes.size());

      /*
      for (std::vector<char>::const_iterator i = fileBytes.begin(); i != fileBytes.end(); ++i) {
        std::cout << *i << ' ';
      }
      */

      /*
      uint8_t* bytes = new uint8_t[fileBytes.size()];
      for (int i = 0; i < fileBytes.size(); i++) {
        bytes[i] = uint8_t(fileBytes[i]);
      }
      */
      
      Local<WasmCompiledModule> module = WasmCompiledModule::DeserializeOrCompile(
        reinterpret_cast<v8::Isolate*>(isolate),
        WasmCompiledModule::BufferReference(0, 0),
        WasmCompiledModule::BufferReference(fileBytes.data(), fileBytes.size())
      ).ToLocalChecked();

      fprintf(stderr, "is Wasm module! '%i'.\n", module->IsWebAssemblyCompiledModule());

      args_type instance_args{module};

      Local<Object> module_instance_exports = context->Global()
          ->Get(context, String::NewFromUtf8(isolate, "WebAssembly"))
          .ToLocalChecked().As<Object>()
          ->Get(context, String::NewFromUtf8(isolate, "Instance"))
          .ToLocalChecked().As<Object>()
          ->CallAsConstructor(context, 1, instance_args)
          .ToLocalChecked().As<Object>()
          ->Get(context, String::NewFromUtf8(isolate, "exports"))
          .ToLocalChecked().As<Object>()
          ;
      /*
      args_type add_args{Int32::New(isolate, 5)};
      Local<Int32> adder_res = module_instance_exports
        ->Get(context, String::NewFromUtf8(isolate, "fib"))
        .ToLocalChecked().As<Function>()
        ->Call(context, context->Global(), 1, add_args)
        .ToLocalChecked().As<Int32>();
      */
      args_type add_args{Int32::New(isolate, 77), Int32::New(isolate, 88)};
      Local<Int32> adder_res = module_instance_exports
        ->Get(context, String::NewFromUtf8(isolate, "add"))
        .ToLocalChecked().As<Function>()
        ->Call(context, context->Global(), 2, add_args)
        .ToLocalChecked().As<Int32>();

      printf("77 + 88 = %d\n", adder_res->Value());
    }

    {
      // taken from: https://github.com/v8/v8/blob/master/samples/hello-world.cc#L66
      std::vector<uint8_t> wasmbin {
              0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00, 0x01, 0x07, 0x01,
              0x60, 0x02, 0x7f, 0x7f, 0x01, 0x7f, 0x03, 0x02, 0x01, 0x00, 0x07,
              0x07, 0x01, 0x03, 0x61, 0x64, 0x64, 0x00, 0x00, 0x0a, 0x09, 0x01,
              0x07, 0x00, 0x20, 0x00, 0x20, 0x01, 0x6a, 0x0b
      };

      // same as calling:
      // let module = new WebAssembly.Module(bytes);
      Local<WasmCompiledModule> module = WasmCompiledModule::DeserializeOrCompile(isolate,
          WasmCompiledModule::BufferReference(0, 0),
          WasmCompiledModule::BufferReference(wasmbin.data(), wasmbin.size())
          ).ToLocalChecked();

      args_type instance_args{module};

      Local<Object> module_instance_exports = context->Global()
          ->Get(context, String::NewFromUtf8(isolate, "WebAssembly"))
          .ToLocalChecked().As<Object>()
          ->Get(context, String::NewFromUtf8(isolate, "Instance"))
          .ToLocalChecked().As<Object>()
          ->CallAsConstructor(context, 1, instance_args)
          .ToLocalChecked().As<Object>()
          ->Get(context, String::NewFromUtf8(isolate, "exports"))
          .ToLocalChecked().As<Object>()
          ;

      args_type add_args{Int32::New(isolate, 77), Int32::New(isolate, 88)};
      Local<Int32> adder_res = module_instance_exports
        ->Get(context, String::NewFromUtf8(isolate, "add"))
        .ToLocalChecked().As<Function>()
        ->Call(context, context->Global(), 2, add_args)
        .ToLocalChecked().As<Int32>();

      printf("77 + 88 = %d\n", adder_res->Value());
    }
  }

  // Dispose the isolate and tear down V8.
  isolate->Dispose();
  v8::V8::Dispose();
  v8::V8::ShutdownPlatform();
  delete create_params.array_buffer_allocator;
  return 0;
}
