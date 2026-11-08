-- Escape sequences of the characters that can't appear as is in a C string
-- literal.
--
local escapes =
{
  ["\\"] = "\\\\",
  ["\""] = "\\\"",
  ["\n"] = "\\n",
  ["\t"] = "\\t",
  ["\r"] = "\\r",
  ["\a"] = "\\a",
  ["\b"] = "\\b"
}

-- Return the string s as a C string literal.
--
local function c_literal (s)
  return "\"" .. s:gsub ("[\\\"\n\t\r\a\b]", escapes) .. "\""
end

return c_literal
