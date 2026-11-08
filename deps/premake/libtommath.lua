-- LibTomMath.
--
local targets = ...

targets:static_library ("libtommath",
{
  directory = "libtommath",
  language  = "C",

  files = {"*.c"},

  export =
  {
    includedirs = {"."},

    defines =
    {
      "LTM_DESC",
      "__STDC_IEC_559__",
      "MP_NO_DEV_URANDOM"
    }
  }
})
