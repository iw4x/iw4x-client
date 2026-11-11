-- Third-party targets of the build and the set that owns them.
--
-- Each script in deps/premake/ describes the targets of one third-party
-- library by declaring them through the target set (see target_set below),
-- one table of settings per target. For example:
--
--   targets:static_library ("zlib",
--   {
--     directory = "zlib",
--     language  = "C",
--     files     = {"*.h", "*.c"},
--
--     export =
--     {
--       includedirs = {"."},
--       defines     = {"ZLIB_CONST"}
--     }
--   })
--
-- The export of a library holds what its consumers compile and link with
-- and the rest of its description holds what it takes to build the library
-- itself.
--
-- Note that the paths in a description are relative to the target
-- directory, which is itself relative to the directory of the set (deps/).
--

-- Make a class, derived from base if specified.
--
local function class (base)
  local c = {}
  c.__index = c
  return setmetatable (c, {__index = base})
end

-- Create an object of the class c and initialize it with the arguments.
--
-- Note that the classes and this function are local to this file, which is
-- how we get the effect of a protected constructor with target_set as its
-- friend: a script can only declare a target through the set, which owns it
-- and makes sure its name is unique.
--
local function new (c, ...)
  local o = setmetatable ({}, c)
  o:init (...)
  return o
end

-- Return true if the object o is of the class c or of a class derived from
-- it.
--
local function is_a (o, c)
  local m = getmetatable (o)

  while m ~= nil do
    if m == c then
      return true
    end

    m = getmetatable (m).__index
  end

  return false
end

-- Settings that a description may specify, each with the premake function
-- it is passed to. A path setting is relative to the target directory and
-- a link setting is what a library export passes to the binaries that link
-- the library. An external setting is passed to its external function when
-- a library export is imported by another target (see
-- library:import_interface()).
--
local setting_types =
{
  warnings     = {apply = warnings},
  buildoptions = {apply = buildoptions, list = true},
  cdialect     = {apply = cdialect},
  cppdialect   = {apply = cppdialect},
  pchheader    = {apply = pchheader},
  pchsource    = {apply = pchsource,   path = true},
  files        = {apply = files,       path = true, list = true},
  removefiles  = {apply = removefiles, path = true, list = true},
  includedirs  = {apply = includedirs, external = externalincludedirs,
                  path = true, list = true},
  defines      = {apply = defines,     list = true},
  libdirs      = {apply = libdirs,     path = true, list = true, link = true},
  links        = {apply = links,       list = true, link = true},
  linkoptions  = {apply = linkoptions, list = true},
  rules        = {apply = rules,       list = true},
  dependson    = {apply = dependson,   list = true}
}

-- Settings of a binary and of a library export, in the order we pass them
-- to premake.
--
local binary_settings =
{
  "warnings",
  "buildoptions",
  "cdialect",
  "cppdialect",
  "pchheader",
  "pchsource",
  "files",
  "removefiles",
  "includedirs",
  "defines",
  "links",
  "linkoptions",
  "rules",
  "dependson"
}

local export_settings =
{
  "includedirs",
  "defines",
  "libdirs",
  "links"
}

-- Description of a target, as written in its script.
--
-- We take each key out of the description as we read it, so whatever is
-- left at the end is a key that the target kind doesn't know about, which
-- verify() diagnoses.
--
local description = class ()

-- Initialize the description from the fields table of its script. The
-- context names the description in diagnostics (for example, "target
-- 'zlib'").
--
function description:init (fields, context)
  if type (fields) ~= "table" then
    premake.error ("expected table instead of %s for %s", type (fields),
                   context)
  end

  self.fields_  = {}
  self.context_ = context

  for k, v in pairs (fields) do
    self.fields_[k] = v
  end
end

-- Issue diagnostics that mention the description and fail.
--
function description:fail (format, ...)
  premake.error ("%s in %s", string.format (format, ...), self.context_)
end

-- Take the key out of the description and return its value, or nil if it
-- is absent. Fail if the value isn't of the kind, which names a Lua type
-- ("string", "table") or is "list" for a list of strings.
--
function description:take (key, kind)
  local v = self.fields_[key]
  self.fields_[key] = nil

  if v == nil then
    return nil
  end

  local valid

  if kind == "list" then
    valid = type (v) == "table"

    for _, x in ipairs (valid and v or {}) do
      valid = valid and type (x) == "string"
    end
  else
    valid = type (v) == kind
  end

  if not valid then
    self:fail ("invalid value of key '%s' (expected %s)", key, kind)
  end

  return v
end

-- Fail if the description has a key that we haven't taken.
--
function description:verify ()
  local k = next (self.fields_)

  if k ~= nil then
    self:fail ("unknown key '%s'", tostring (k))
  end
end

-- Settings read from a description, passed to premake for the project
-- being defined.
--
local settings = class ()

-- Take the settings named in names out of the description d. Their paths
-- are relative to the directory.
--
function settings:init (d, directory, names)
  self.values_ = {} -- Setting type and value, in the names order.

  for _, n in ipairs (names) do
    local t = setting_types[n]
    local v = d:take (n, t.list and "list" or "string")

    if v ~= nil then
      if t.path then
        local complete = function (p)
          return path.normalize (path.join (directory, p))
        end

        if t.list then
          local ps = {}

          for _, p in ipairs (v) do
            table.insert (ps, complete (p))
          end

          v = ps
        else
          v = complete (v)
        end
      end

      table.insert (self.values_, {type = t, value = v})
    end
  end
end

-- Pass the settings selected by the predicate to their premake functions.
--
function settings:apply_if (predicate)
  for _, s in ipairs (self.values_) do
    if predicate (s.type) then
      s.type.apply (s.value)
    end
  end
end

function settings:apply ()
  self:apply_if (function (t) return true end)
end

-- Pass the settings other than the link ones to their premake functions. If
-- external is true, then pass each to its external function if it has one.
--
function settings:apply_compile (external)
  for _, s in ipairs (self.values_) do
    local t = s.type

    if not t.link then
      local f = external and t.external or t.apply
      f (s.value)
    end
  end
end

function settings:apply_link ()
  self:apply_if (function (t) return t.link end)
end

-- Binary that a target builds: its project settings and the libraries it
-- depends on.
--
local binary = class ()

function binary:init (set, d, directory)
  self.set_          = set
  self.language_     = d:take ("language", "string") or
                       d:fail ("no language specified")
  self.dependencies_ = d:take ("dependencies", "list") or {}
  self.settings_     = new (settings, d, directory, binary_settings)
end

-- Return the names of the libraries the binary depends on.
--
function binary:dependencies ()
  return self.dependencies_
end

-- Define the project of the binary with the premake kind k. If linked is
-- true, then the binary is linked and links its dependencies along with
-- everything they depend on. If owner is specified, then it is the library
-- that the binary is of, and the binary also compiles with its export.
--
function binary:define (name, k, linked, owner)
  project (name)
    kind (k)
    language (self.language_)

    -- Turn the warnings off for third-party code since they are for
    -- upstream to fix. A description can turn them back on (see
    -- iw4-open-formats.lua).
    --
    warnings "Off"

    if owner ~= nil then
      owner:import_interface ({}, false)
    end

    self.settings_:apply ()

    if linked then
      self.set_:import (self.dependencies_, name)
    else
      self.set_:import_interface (self.dependencies_, name)
    end
end

-- Target declared by a description script.
--
-- This is the abstract base of the target kinds below, each of which
-- implements define() to define its project, if it has one.
--
local target = class ()

function target:init (set, name, d)
  self.set_       = set
  self.name_      = name
  self.directory_ = path.join (set:directory (),
                               d:take ("directory", "string") or
                               d:fail ("no directory specified"))
end

function target:name ()
  return self.name_
end

-- Library, which other targets and the client import.
--
-- This is the abstract base of the library kinds. A library that builds a
-- binary implements link() to link it and dependencies() to return what it
-- depends on.
--
local library = class (target)

-- Note that the export may name the libraries that the interface of this
-- one depends on (for example, those whose headers it includes). Their
-- exports become part of this one.
--
function library:init (set, name, d)
  target.init (self, set, name, d)

  local e = new (description,
                 d:take ("export", "table") or {},
                 "export of target '" .. name .. "'")

  self.export_    = new (settings, e, self.directory_, export_settings)
  self.interface_ = e:take ("dependencies", "list") or {}

  e:verify ()
end

function library:link ()
end

function library:dependencies ()
  return {}
end

-- Add the export of this library and of its interface dependencies to the
-- project being defined. The seen table holds the libraries that are
-- already added. If external is true, then the project is of another
-- target and gets the include directories as external ones.
--
-- Note that the compilers don't warn about the headers they find in the
-- external include directories (with GCC they are -isystem directories).
-- This way we see the warnings of a library only when we build the library
-- itself, which imports its own export as non-external.
--
function library:import_interface (seen, external)
  if seen[self] then
    return
  end

  seen[self] = true

  self.export_:apply_compile (external)

  for _, n in ipairs (self.interface_) do
    self.set_:find (n, self.name_):import_interface (seen, true)
  end
end

-- Link this library along with everything it depends on into the project
-- being defined. The seen table holds the libraries that are already
-- linked.
--
function library:import_link (seen)
  if seen[self] then
    return
  end

  seen[self] = true

  self:link ()
  self.export_:apply_link ()

  for _, ns in ipairs ({self.interface_, self:dependencies ()}) do
    for _, n in ipairs (ns) do
      self.set_:find (n, self.name_):import_link (seen)
    end
  end
end

-- Library without a binary (headers, an SDK with prebuilt libraries, etc).
--
local binless_library = class (library)

function binless_library:define ()
end

-- Static library.
--
-- Note that a static library doesn't link the libraries it depends on: it
-- compiles against their exports, and whichever binary links it links them
-- as well (see library:import_link()).
--
local static_library = class (library)

function static_library:init (set, name, d)
  library.init (self, set, name, d)
  self.binary_ = new (binary, set, d, self.directory_)
end

function static_library:define ()
  self.binary_:define (self.name_, "StaticLib", false, self)
end

function static_library:link ()
  links {self.name_}
end

function static_library:dependencies ()
  return self.binary_:dependencies ()
end

local executable = class (target)

function executable:init (set, name, d)
  target.init (self, set, name, d)
  self.binary_ = new (binary, set, d, self.directory_)
end

function executable:define ()
  self.binary_:define (self.name_, "ConsoleApp", true)
end

-- Shared library that is loaded at runtime (a plugin). Since nothing links
-- it, it isn't a library in the import sense and has no export.
--
local shared_library = class (target)

function shared_library:init (set, name, d)
  target.init (self, set, name, d)
  self.binary_ = new (binary, set, d, self.directory_)
end

function shared_library:define ()
  self.binary_:define (self.name_, "SharedLib", true)
end

-- Set of the targets declared by the description scripts.
--
local target_set = {}
target_set.__index = target_set

-- Create an empty set whose target directories are relative to the
-- directory, which must be absolute.
--
function target_set.new (directory)
  local s = setmetatable ({}, target_set)
  s.directory_ = directory
  s.targets_   = {} -- Targets by name.
  return s
end

function target_set:directory ()
  return self.directory_
end

-- Declare a target of the class c in the set s.
--
local function declare (s, c, name, fields)
  if s.targets_[name] ~= nil then
    premake.error ("duplicate target '%s'", name)
  end

  local d = new (description, fields, "target '" .. name .. "'")
  local t = new (c, s, name, d)

  d:verify ()

  s.targets_[name] = t
end

function target_set:binless_library (name, fields)
  declare (self, binless_library, name, fields)
end

function target_set:static_library (name, fields)
  declare (self, static_library, name, fields)
end

function target_set:executable (name, fields)
  declare (self, executable, name, fields)
end

function target_set:shared_library (name, fields)
  declare (self, shared_library, name, fields)
end

-- Run each description script in the directory, passing the set as its
-- argument.
--
function target_set:load (directory)
  local fs = os.matchfiles (path.join (directory, "*.lua"))
  table.sort (fs)

  for _, f in ipairs (fs) do
    local script, e = loadfile (f)

    if script == nil then
      premake.error ("unable to load %s: %s", f, e)
    end

    script (self)
  end
end

-- Return the library named name, which the target named user depends on.
-- Fail if there is no such library.
--
function target_set:find (name, user)
  local t = self.targets_[name]

  if t == nil then
    premake.error ("unknown library '%s' in dependencies of '%s'", name,
                   user)
  end

  if not is_a (t, library) then
    premake.error ("target '%s' in dependencies of '%s' is not a library",
                   name, user)
  end

  return t
end

-- Define the projects of the targets, in the name order to keep the
-- generated solution stable.
--
function target_set:define ()
  local ns = {}

  for n in pairs (self.targets_) do
    table.insert (ns, n)
  end

  table.sort (ns)

  for _, n in ipairs (ns) do
    self.targets_[n]:define ()
  end
end

-- Import the libraries named in names into the project of the linked
-- binary named user: add their exports and link them along with everything
-- they depend on.
--
function target_set:import (names, user)
  local interface = {}
  local link      = {}

  for _, n in ipairs (names) do
    local l = self:find (n, user)

    l:import_interface (interface, true)
    l:import_link (link)
  end
end

-- Import the exports of the libraries named in names into the project of
-- the static library named user.
--
function target_set:import_interface (names, user)
  local interface = {}

  for _, n in ipairs (names) do
    self:find (n, user):import_interface (interface, true)
  end
end

return target_set
