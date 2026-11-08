-- RapidJSON.
--
-- Note that each consumer decides whether to enable the std::string
-- support (RAPIDJSON_HAS_STDSTRING). iw4-open-formats can't, since it
-- provides its own.
--
local targets = ...

targets:binless_library ("rapidjson",
{
  directory = "rapidjson",

  export =
  {
    includedirs = {"include"}
  }
})
