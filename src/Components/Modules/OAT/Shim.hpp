#pragma once

#include <cstddef>
#include <functional>
#include <memory>
#include <optional>
#include <string>

// Compile raw menu files with OpenAssetTools.
//
// On x86 IW4x loads the raw menus in our iwds through the game's own menu
// parser, the PC_*() and Menu_Parse() family. x64 compiled all of that
// out. Presumably the later revision only ever loads menus from zones, so
// to the game it's just dead code. To us it means that every raw menu we
// ship is quietly ignored and the game falls back to its stock ones, the
// main menu included.
//
// So what can we do? One option would be to bring the parser back by
// porting x86's, function by function. But the parser comes with its own
// precompiler and expression compiler, which is a good chunk of code that
// we would then have to keep in step with what the game expects. Another
// is to convert our menus into a zone at build time. But then a mod's
// menus on disk would stop working, which x86 supports and people rely
// on. What we went with instead is OpenAssetTools (OAT), which already
// has a menu compiler for IW4 that produces the game's menu structures,
// the same ones a zone would hold. We have OAT compile the file as the
// game's file system finds it, and then hand the menus over to Menus,
// which does the rest the same way as x86.
//
// The catch is that OAT builds with include directories and definitions
// that the client doesn't have. So the oat target compiles the shim (see
// deps/premake/oat.lua) and this header is the only thing the two sides
// share. That's why it only declares the few game types it mentions and
// includes nothing from OAT.
//
// It also means that neither side can see both definitions of a menu
// structure, so neither can check by itself that OAT lays them out the
// way the game does. If they ever disagree, the game would read OAT's
// menus a few bytes off, which presumably makes for some very creative
// crashes. So we check them in two halves: Shim.cpp checks OAT's types
// against the sizes and offsets below and Menus.cpp checks the game's
// against the same numbers.
//
// Compile raw weapon files the same way.
//
// On x86 the game parses a weapon's raw file (weapons/mp/<name>) with its
// info string parser, which x64 compiled out along with the menu parser.
// OAT has an IW4 weapon loader too, so we let it do the parsing. Its
// weapon is the one a zone would hold, though, so a few fields still name
// what the game keeps resolved (the sounds) or index OAT's own list of
// script strings. LoadWeaponFile() resolves them through the callbacks
// before handing the weapon over, the way the game's zone loader does for
// the weapons of a zone.
//
namespace Game
{
  struct MenuList;
  struct Material;
  struct snd_alias_list_t;
  struct WeaponCompleteDef;
}

namespace Components::OAT
{
  // The menus of a menu (.menu) or menu list (.txt) file, as OAT compiled
  // them.
  //
  struct MenuFile
  {
    Game::MenuList* list;

    // The memory that the menus live in, which stays valid for as long as
    // someone holds on to it. It's OAT's zone underneath, but the client has
    // no business knowing that, hence the void.
    //
    std::shared_ptr<void> keepalive;
  };

  // The kinds of messages OAT prints while it compiles a file.
  //
  enum class Message
  {
    debug,
    info,
    warning,
    error
  };

  struct MenuCallbacks
  {
    // Read the file through the game's file system, or return nullopt if it
    // doesn't exist. OAT opens both the menu file and every file it includes
    // through this, which is how it finds them in the iwds and the mod, the
    // same as the game's parser does on x86.
    //
    std::function<std::optional<std::string> (const std::string& file)> read;

    // Find a material or a sound that the menus refer to, which the game's
    // parser looks up in the asset database as well.
    //
    std::function<Game::Material* (const std::string& name)> material;
    std::function<Game::snd_alias_list_t* (const std::string& name)> sound;

    // Print a message that OAT has for us (see con in Shim.cpp).
    //
    std::function<void (Message, const std::string&)> print;
  };

  // Compile the menu or menu list file, or return nullopt if it doesn't
  // exist or doesn't compile. Whatever went wrong will have been printed
  // through the callbacks by then.
  //
  std::optional<MenuFile> LoadMenuFile(const std::string& file, const MenuCallbacks& callbacks);

  // The weapon of a raw weapon file, as OAT compiled it, with the same
  // keepalive as a menu file.
  //
  struct WeaponFile
  {
    Game::WeaponCompleteDef* weapon;

    std::shared_ptr<void> keepalive;
  };

  struct WeaponCallbacks
  {
    // Read the file through the game's file system, as for a menu file.
    //
    std::function<std::optional<std::string> (const std::string& file)> read;

    // Find an asset of the type that the weapon refers to, or return null if
    // there is none. The type is numbered the way the game numbers its
    // asset types, which is the same as OAT's IW4 numbering.
    //
    std::function<void* (int type, const std::string& name)> find;

    // Return the game's script string for the text.
    //
    std::function<unsigned short (const char* text)> string;

    std::function<void (Message, const std::string&)> print;
  };

  // Compile the raw weapon file into the weapon of the name, or return
  // nullopt if there is no such file or it doesn't compile.
  //
  std::optional<WeaponFile> LoadWeaponFile(const std::string& file, const std::string& name, const WeaponCallbacks& callbacks);
  // The layout of the menu structures as OAT compiles them, which has to be
  // the game's (see above). These are the sizes of the structures a compiled
  // menu is made of, plus the offsets of a few fields we reach into. It's not
  // every field, though, so two layouts that differ inside a structure of
  // the same size would still slip through.
  //
  namespace Layout
  {
    constexpr std::size_t windowDef_t = 176;
    constexpr std::size_t menuDef_t = 488;
    constexpr std::size_t menuDef_t_items = 360;
    constexpr std::size_t itemDef_s = 488;
    constexpr std::size_t itemDef_s_parent = 256;
    constexpr std::size_t itemDef_s_typeData = 392;
    constexpr std::size_t Statement_s = 48;
    constexpr std::size_t Statement_s_supportingData = 16;
    constexpr std::size_t ExpressionSupportingData = 48;
    constexpr std::size_t MenuEventHandlerSet = 16;
    constexpr std::size_t MenuEventHandler = 16;
    constexpr std::size_t ItemKeyHandler = 24;
    constexpr std::size_t MenuList = 24;
    constexpr std::size_t listBoxDef_s = 336;
    constexpr std::size_t multiDef_s = 648;

    // The weapon structures, which Weapon.cpp checks the game's side of.
    // The game's own sizes, which we got from x64's zone loader, are the
    // same.
    //
    constexpr std::size_t WeaponCompleteDef = 0xA0;
    constexpr std::size_t WeaponDef = 0x890;
  }
}
