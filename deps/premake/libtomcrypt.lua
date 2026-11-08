-- LibTomCrypt, with LibTomMath as its math provider.
--
local targets = ...

targets:static_library ("libtomcrypt",
{
  directory = "libtomcrypt",
  language  = "C",

  files = {"src/**.c"},

  removefiles =
  {
    "src/**/*_test.c",
    "src/**/*tab.c",        -- Included by the sources that use them.
    "src/encauth/ocb3/**.c" -- @@ Why is OCB3 left out?
  },

  defines =
  {
    "_CRT_SECURE_NO_WARNINGS",
    "LTC_SOURCE",
    "USE_LTM"
  },

  export =
  {
    includedirs  = {"src/headers"},
    dependencies = {"libtommath"},

    -- Note that these change the declarations in the headers, so the
    -- library and its consumers must agree on them.
    --
    defines =
    {
      "LTC_NO_FAST",
      "LTC_NO_PROTOTYPES",
      "LTC_NO_RSA_BLINDING",
      "LTC_NO_FILE",
      "ARGTYPE=4"
    }
  }
})
