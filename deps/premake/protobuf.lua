-- Protocol Buffers runtime along with the client messages.
--
-- Note that the ProtobufCompiler rule (see premake5.lua) compiles the
-- client messages (src/Proto/*.proto) into this library and writes the
-- generated sources into the build directory.
--
local targets = ...

targets:static_library ("protobuf",
{
  directory = "protobuf",
  language  = "C++",
  rules     = {"ProtobufCompiler"},

  files =
  {
    "src/**.cc",
    path.join (_MAIN_SCRIPT_DIR, "src/**.proto")
  },

  -- Leave out the tests and the compiler along with error_listener.cc,
  -- which has no definitions.
  --
  removefiles =
  {
    "src/**/*test.cc",
    "src/google/protobuf/*test*.cc",
    "src/google/protobuf/testing/**.cc",
    "src/google/protobuf/compiler/**.cc",
    "src/google/protobuf/util/internal/error_listener.cc"
  },

  export =
  {
    includedirs = {"src"}
  }
})
