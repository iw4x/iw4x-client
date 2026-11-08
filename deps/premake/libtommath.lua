-- LibTomMath.
--
local targets = ...

targets:static_library ("libtommath",
{
  directory = "libtommath",
  language  = "C",

  files = {"*.c"},

  -- As in libtomcrypt.
  --
  linkoptions = {"-IGNORE:4221"},

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
