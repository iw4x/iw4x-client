-- OpenAssetTools, of which we build the IW4 menu compiler and weapon loader
-- that we load the raw menus and weapons with (see
-- src/Components/Modules/OAT/).
--
-- Note that OAT needs C++23, so we also compile the shim between it and
-- the client here (see Shim.cpp), behind a header that is C++20. The shim
-- also takes the place of OAT's console log and of IParserLineStream.cpp.
--
-- Note also that OAT templates its menu converter for each IW4 flavor at
-- its own build time (see src/ObjLoading/Menu/*.template). We expand the
-- IW4 one here into the build directory, which only takes the #set and
-- #include directives and the GAME token that the templates use.
--
local targets = ...

local oat_directory = path.join (_MAIN_SCRIPT_DIR, "deps/oat")
local templated_directory = path.join (_MAIN_SCRIPT_DIR, "build",
                                       _OPTIONS["mingw"] and "mingw" or "",
                                       "src/oat")

-- Return the value of the #set expression, which concatenates string
-- literals and GAME with +.
--
local function evaluate (expression, game)
  local r = ""
  local i = 1

  while i <= #expression do
    local c = expression:sub (i, i)

    if c == '"' then
      i = i + 1

      while expression:sub (i, i) ~= '"' do
        if expression:sub (i, i) == "\\" then
          i = i + 1
        end

        r = r .. expression:sub (i, i)
        i = i + 1
      end

      i = i + 1
    elseif expression:find ("^GAME", i) then
      r = r .. game
      i = i + 4
    else
      i = i + 1
    end
  end

  return r
end

-- Expand the template into the file, leaving the file alone if it already
-- has the result so that we don't rebuild what includes it.
--
local function expand (template, file, game)
  local sets = {}
  local lines = {}

  for line in (io.readfile (template) .. "\n"):gmatch ("(.-)\r?\n") do
    if line:find ("^#options") or line:find ("^#filename") then
    elseif line:find ("^#set ") then
      local name, expression = line:match ("^#set ([%w_]+) (.*)$")
      sets[name] = evaluate (expression, game)
    elseif line:find ("^#include [%w_]+$") then
      local name = line:match ("^#include ([%w_]+)$")

      if sets[name] == nil then
        premake.error ("undefined template variable %s in %s", name, template)
      end

      table.insert (lines, "#include " .. sets[name])
    else
      table.insert (lines, (line:gsub ("%f[%w_]GAME%f[^%w_]", game)))
    end
  end

  local result = table.concat (lines, "\n")

  if io.readfile (file) ~= result then
    os.mkdir (path.getdirectory (file))

    if not io.writefile (file, result) then
      premake.error ("unable to write %s", file)
    end
  end
end

for _, e in ipairs ({"h", "cpp"}) do
  expand (path.join (oat_directory,
                     "src/ObjLoading/Menu/MenuConverter." .. e .. ".template"),
          path.join (templated_directory,
                     "ObjLoading/Game/IW4/Menu/MenuConverterIW4." .. e),
          "IW4")
end

targets:static_library ("oat",
{
  directory  = "oat",
  language   = "C++",
  cppdialect = "C++23",

  -- Note that the parser of the menu expressions dynamic_casts them, while
  -- we turn RTTI off for Release (see premake5.lua).
  --
  rtti = "On",

  -- Note that some of the parser sources have more sections than an object
  -- file holds by default, which is why OAT builds with /bigobj too.
  --
  buildoptions = _OPTIONS["mingw"] and {"-Wa,-mbig-obj"} or {"/bigobj"},

  -- Only what the IW4 menu compiler and weapon loader need. Note that we
  -- also need the descriptions of the other games, which IGame.cpp lists
  -- along with IW4's.
  --
  files =
  {
    "src/Common/Game/IGame.cpp",
    "src/Common/Game/IW3/GameIW3.cpp",
    "src/Common/Game/IW4/GameIW4.cpp",
    "src/Common/Game/IW4MS/GameIW4MS.cpp",
    "src/Common/Game/IW5/GameIW5.cpp",
    "src/Common/Game/QOS/GameQOS.cpp",
    "src/Common/Game/T4/GameT4.cpp",
    "src/Common/Game/T5/GameT5.cpp",
    "src/Common/Game/T6/GameT6.cpp",
    "src/ObjCommon/InfoString/InfoString.cpp",
    "src/ObjCommon/SearchPath/ISearchPath.cpp",
    "src/ObjCommon/SearchPath/SearchPathFilesystem.cpp",
    "src/ObjCommon/SearchPath/SearchPathMultiInputStream.cpp",
    "src/ObjCommon/SearchPath/SearchPaths.cpp",
    "src/ObjCommon/SearchPath/SearchPathSearchOptions.cpp",
    "src/ObjCommon/Weapon/WeaponCommon.cpp",
    "src/ObjLoading/Asset/AssetCreationContext.cpp",
    "src/ObjLoading/Asset/AssetCreationResult.cpp",
    "src/ObjLoading/Asset/AssetCreatorCollection.cpp",
    "src/ObjLoading/Game/IW4/InfoString/InfoStringToStructConverter.cpp",
    "src/ObjLoading/Game/IW4/Menu/LoaderMenuListIW4.cpp",
    "src/ObjLoading/Game/IW4/Menu/MenuConversionZoneStateIW4.cpp",
    "src/ObjLoading/Game/IW4/Weapon/InfoStringLoaderWeaponIW4.cpp",
    "src/ObjLoading/InfoString/InfoStringToStructConverterBase.cpp",
    "src/ObjLoading/Menu/AbstractMenuConverter.cpp",
    "src/ObjLoading/Menu/MenuConversionException.cpp",
    "src/ObjLoading/ObjLoading.cpp",
    "src/ObjLoading/Parsing/Menu/Domain/CommonLayout.cpp",
    "src/ObjLoading/Parsing/Menu/Domain/CommonMenuTypes.cpp",
    "src/ObjLoading/Parsing/Menu/Domain/EventHandler/CommonEventHandlerCondition.cpp",
    "src/ObjLoading/Parsing/Menu/Domain/EventHandler/CommonEventHandlerScript.cpp",
    "src/ObjLoading/Parsing/Menu/Domain/EventHandler/CommonEventHandlerSet.cpp",
    "src/ObjLoading/Parsing/Menu/Domain/EventHandler/CommonEventHandlerSetLocalVar.cpp",
    "src/ObjLoading/Parsing/Menu/Domain/Expression/CommonExpressionBaseFunctionCall.cpp",
    "src/ObjLoading/Parsing/Menu/Domain/Expression/CommonExpressionCustomFunctionCall.cpp",
    "src/ObjLoading/Parsing/Menu/Matcher/MenuExpressionMatchers.cpp",
    "src/ObjLoading/Parsing/Menu/Matcher/MenuMatcherFactory.cpp",
    "src/ObjLoading/Parsing/Menu/Matcher/MenuMatcherScriptInt.cpp",
    "src/ObjLoading/Parsing/Menu/Matcher/MenuMatcherScriptNumeric.cpp",
    "src/ObjLoading/Parsing/Menu/MenuAssetZoneState.cpp",
    "src/ObjLoading/Parsing/Menu/MenuFileCommonOperations.cpp",
    "src/ObjLoading/Parsing/Menu/MenuFileParser.cpp",
    "src/ObjLoading/Parsing/Menu/MenuFileParserState.cpp",
    "src/ObjLoading/Parsing/Menu/MenuFileReader.cpp",
    "src/ObjLoading/Parsing/Menu/Sequence/EventHandlerSetScopeSequences.cpp",
    "src/ObjLoading/Parsing/Menu/Sequence/FunctionScopeSequences.cpp",
    "src/ObjLoading/Parsing/Menu/Sequence/Generic/GenericBoolPropertySequence.cpp",
    "src/ObjLoading/Parsing/Menu/Sequence/Generic/GenericColorPropertySequence.cpp",
    "src/ObjLoading/Parsing/Menu/Sequence/Generic/GenericExpressionPropertySequence.cpp",
    "src/ObjLoading/Parsing/Menu/Sequence/Generic/GenericFloatingPointPropertySequence.cpp",
    "src/ObjLoading/Parsing/Menu/Sequence/Generic/GenericIntPropertySequence.cpp",
    "src/ObjLoading/Parsing/Menu/Sequence/Generic/GenericKeywordPropertySequence.cpp",
    "src/ObjLoading/Parsing/Menu/Sequence/Generic/GenericMenuEventHandlerSetPropertySequence.cpp",
    "src/ObjLoading/Parsing/Menu/Sequence/Generic/GenericStringPropertySequence.cpp",
    "src/ObjLoading/Parsing/Menu/Sequence/GlobalScopeSequences.cpp",
    "src/ObjLoading/Parsing/Menu/Sequence/ItemScopeSequences.cpp",
    "src/ObjLoading/Parsing/Menu/Sequence/LayoutSequences.cpp",
    "src/ObjLoading/Parsing/Menu/Sequence/MenuScopeSequences.cpp",
    "src/ObjLoading/Parsing/Menu/Sequence/NoScopeSequences.cpp",
    "src/Parser/Parsing/Impl/AbstractDirectiveStreamProxy.cpp",
    "src/Parser/Parsing/Impl/CommentRemovingStreamProxy.cpp",
    "src/Parser/Parsing/Impl/DefinesStreamProxy.cpp",
    "src/Parser/Parsing/Impl/IncludingStreamProxy.cpp",
    "src/Parser/Parsing/Impl/PackDefinitionStreamProxy.cpp",
    "src/Parser/Parsing/Impl/ParserFilesystemStream.cpp",
    "src/Parser/Parsing/Impl/ParserMultiInputStream.cpp",
    "src/Parser/Parsing/Impl/ParserSingleInputStream.cpp",
    "src/Parser/Parsing/Impl/SkipUntilFirstNonEmptyProxy.cpp",
    "src/Parser/Parsing/Matcher/MatcherResult.cpp",
    "src/Parser/Parsing/ParsingException.cpp",
    "src/Parser/Parsing/Simple/Expression/SimpleExpressionBinaryOperation.cpp",
    "src/Parser/Parsing/Simple/Expression/SimpleExpressionConditionalOperator.cpp",
    "src/Parser/Parsing/Simple/Expression/SimpleExpressionMatchers.cpp",
    "src/Parser/Parsing/Simple/Expression/SimpleExpressionScopeValue.cpp",
    "src/Parser/Parsing/Simple/Expression/SimpleExpressionUnaryOperation.cpp",
    "src/Parser/Parsing/Simple/Expression/SimpleExpressionValue.cpp",
    "src/Parser/Parsing/Simple/Matcher/SimpleMatcherAnyCharacterBesides.cpp",
    "src/Parser/Parsing/Simple/Matcher/SimpleMatcherCharacter.cpp",
    "src/Parser/Parsing/Simple/Matcher/SimpleMatcherFactory.cpp",
    "src/Parser/Parsing/Simple/Matcher/SimpleMatcherKeyword.cpp",
    "src/Parser/Parsing/Simple/Matcher/SimpleMatcherKeywordIgnoreCase.cpp",
    "src/Parser/Parsing/Simple/Matcher/SimpleMatcherKeywordPrefix.cpp",
    "src/Parser/Parsing/Simple/Matcher/SimpleMatcherMultiCharacter.cpp",
    "src/Parser/Parsing/Simple/Matcher/SimpleMatcherValueTypeAndHasPrefix.cpp",
    "src/Parser/Parsing/Simple/Matcher/SimpleMatcherValueType.cpp",
    "src/Parser/Parsing/Simple/SimpleExpressionInterpreter.cpp",
    "src/Parser/Parsing/Simple/SimpleLexer.cpp",
    "src/Parser/Parsing/Simple/SimpleParserValue.cpp",
    "src/Parser/Parsing/TokenPos.cpp",
    "src/Utils/Utils/MemoryManager.cpp",
    "src/Utils/Utils/StringUtils.cpp",
    "src/ZoneCommon/Pool/AssetPool.cpp",
    "src/ZoneCommon/Pool/GlobalAssetPool.cpp",
    "src/ZoneCommon/Pool/XAssetInfo.cpp",
    "src/ZoneCommon/Zone/XBlock.cpp",
    "src/ZoneCommon/Zone/Zone.cpp",
    "src/ZoneCommon/Zone/ZoneMemory.cpp",
    "src/ZoneCommon/Zone/ZoneRegistry.cpp",
    "src/ZoneCommon/Zone/ZoneScriptStrings.cpp",
    path.join (templated_directory,
               "ObjLoading/Game/IW4/Menu/MenuConverterIW4.cpp"),
    path.join (_MAIN_SCRIPT_DIR, "src/Components/Modules/OAT/Shim.cpp")
  },

  includedirs =
  {
    "src/Common",
    "src/ObjCommon",
    "src/ObjLoading",
    "src/Parser",
    "src/Utils",
    "src/ZoneCommon",
    path.join (templated_directory, "ObjLoading")
  },

  defines =
  {
    "ARCH_x64",
    "_CRT_SECURE_NO_WARNINGS",
    "__STDC_LIB_EXT1__",
    "__STDC_WANT_LIB_EXT1__=1"
  },

  -- Note that our debug build defines DEBUG, which OAT has as the name of a
  -- log level (see src/Utils/Utils/Logging/Log.h). So we take it back for
  -- OAT, which doesn't use it otherwise.
  --
  undefines = {"DEBUG"},

  export = {}
})
