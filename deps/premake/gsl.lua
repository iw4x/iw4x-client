-- Guidelines Support Library.
--
local targets = ...

targets:binless_library ("gsl",
{
  directory = "GSL",

  export =
  {
    includedirs = {"include"}
  }
})
