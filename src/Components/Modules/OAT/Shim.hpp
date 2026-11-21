#pragma once

#include <cstddef>
#include <functional>
#include <memory>
#include <optional>
#include <string>

// Note that we include this into both the client and the OpenAssetTools
// shim (see Shim.cpp), which is C++23 and doesn't see the game headers. So
// we only declare the game types here and keep to what C++20 has.
//
namespace Game
{
  struct MenuList;
  struct Material;
  struct snd_alias_list_t;
}

namespace Components::OAT
{
  // The menus of a menu or menu list file, compiled by OpenAssetTools.
  //
  struct MenuFile
  {
    Game::MenuList* list;

    // The memory that the menus live in, which stays valid for as long as
    // we hold on to it.
    //
    std::shared_ptr<void> keepalive;
  };

  enum class MenuMessage
  {
    debug,
    info,
    warning,
    error
  };

  struct MenuCallbacks
  {
    // Read the file through the game's filesystem, or return nullopt if it
    // doesn't exist. The compiler opens the menu file and the files it
    // includes through this.
    //
    std::function<std::optional<std::string> (const std::string& file)> read;

    // Find the material or sound that the menus refer to, as the game's
    // menu parser does.
    //
    std::function<Game::Material* (const std::string& name)> material;
    std::function<Game::snd_alias_list_t* (const std::string& name)> sound;

    std::function<void (MenuMessage, const std::string&)> print;
  };

  // Compile the menu (.menu) or menu list (.txt) file, or return nullopt if
  // it doesn't exist or doesn't compile.
  //
  std::optional<MenuFile> LoadMenuFile(const std::string& file, const MenuCallbacks& callbacks);

  // The layout of the menu structures as OpenAssetTools compiles them, which
  // has to be the game's (see Menus.cpp).
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
  }
}
