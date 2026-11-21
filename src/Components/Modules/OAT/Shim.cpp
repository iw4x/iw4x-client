#include "Shim.hpp"

#include <algorithm>
#include <atomic>
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <sstream>
#include <string_view>
#include <utility>

#include <Asset/AssetCreationContext.h>
#include <Asset/AssetCreationResult.h>
#include <Asset/AssetCreatorCollection.h>
#include <Asset/IAssetCreator.h>
#include <Game/IW4/IW4.h>
#include <Game/IW4/Menu/LoaderMenuListIW4.h>
#include <Game/IW4/MenuConstantsIW4.h>
#include <ObjLoading.h>
#include <Parsing/IParserLineStream.h>
#include <Pool/XAssetInfo.h>
#include <SearchPath/ISearchPath.h>
#include <Utils/Logging/Log.h>
#include <Zone/Zone.h>

// Note that the client includes Shim.hpp without the OpenAssetTools headers
// and so can't check that the game's structures are laid out the way we
// compile them. So we check our side against the layout it names and the
// client checks the game's (see Menus.cpp).
//
namespace Components::OAT::Layout
{
  static_assert(sizeof(IW4::windowDef_t) == windowDef_t);
  static_assert(sizeof(IW4::menuDef_t) == menuDef_t);
  static_assert(offsetof(IW4::menuDef_t, items) == menuDef_t_items);
  static_assert(sizeof(IW4::itemDef_s) == itemDef_s);
  static_assert(offsetof(IW4::itemDef_s, parent) == itemDef_s_parent);
  static_assert(offsetof(IW4::itemDef_s, typeData) == itemDef_s_typeData);
  static_assert(sizeof(IW4::Statement_s) == Statement_s);
  static_assert(offsetof(IW4::Statement_s, supportingData) == Statement_s_supportingData);
  static_assert(sizeof(IW4::ExpressionSupportingData) == ExpressionSupportingData);
  static_assert(sizeof(IW4::MenuEventHandlerSet) == MenuEventHandlerSet);
  static_assert(sizeof(IW4::MenuEventHandler) == MenuEventHandler);
  static_assert(sizeof(IW4::ItemKeyHandler) == ItemKeyHandler);
  static_assert(sizeof(IW4::MenuList) == MenuList);
  static_assert(sizeof(IW4::listBoxDef_s) == listBoxDef_s);
  static_assert(sizeof(IW4::multiDef_s) == multiDef_s);
}

namespace
{
  using namespace Components::OAT;

  // The callbacks of the file we are compiling, which OpenAssetTools prints
  // its messages through (see con below).
  //
  thread_local const MenuCallbacks* callbacks = nullptr;

  void Print(const MenuMessage message, const std::string& text)
  {
    if (callbacks != nullptr && callbacks->print)
    {
      callbacks->print(message, text);
    }
  }

  // Return true if the name is that of a function of the menu expressions,
  // such as sin or milliseconds.
  //
  bool IsExpressionFunction(const std::string_view name)
  {
    for (const auto* function : IW4::g_expFunctionNames)
    {
      const std::string_view f(function);

      if (f.size() == name.size() &&
          std::equal(f.begin(), f.end(), name.begin(), [](const char a, const char b)
          {
            return std::tolower(static_cast<unsigned char>(a)) == std::tolower(static_cast<unsigned char>(b));
          }))
      {
        return true;
      }
    }

    return false;
  }

  // Note that the menus that we dumped from the game's zones write the
  // parenthesis that opens the argument of a function before its name, as
  // in ( sin milliseconds( ) / 1500 ) for sin( milliseconds( ) / 1500 ). The
  // game takes the name of a function for the parenthesis that opens it,
  // while OpenAssetTools expects one after the name.
  //
  // So we move such a parenthesis after the name, which is where it opens
  // the argument for the game as well. We leave the strings and comments of
  // the file alone.
  //
  std::string MoveFunctionParentheses(const std::string& text)
  {
    std::string r;
    r.reserve(text.size());

    const auto n(text.size());
    const auto blank([&text, n](std::size_t i)
    {
      while (i < n && (text[i] == ' ' || text[i] == '\t'))
      {
        ++i;
      }

      return i;
    });

    for (std::size_t i = 0; i < n;)
    {
      const auto c(text[i]);

      if (c == '"')
      {
        auto e(i + 1);

        while (e < n && text[e] != '"' && text[e] != '\n')
        {
          e += text[e] == '\\' ? 2 : 1;
        }

        e = std::min(e + 1, n);
        r.append(text, i, e - i);
        i = e;
      }
      else if (c == '/' && i + 1 < n && text[i + 1] == '/')
      {
        auto e(text.find('\n', i));
        e = e == std::string::npos ? n : e;

        r.append(text, i, e - i);
        i = e;
      }
      else if (c == '/' && i + 1 < n && text[i + 1] == '*')
      {
        auto e(text.find("*/", i + 2));
        e = e == std::string::npos ? n : e + 2;

        r.append(text, i, e - i);
        i = e;
      }
      else if (c == '(')
      {
        const auto b(blank(i + 1));
        auto e(b);

        while (e < n && (std::isalnum(static_cast<unsigned char>(text[e])) || text[e] == '_'))
        {
          ++e;
        }

        const auto next(blank(e));

        if (e != b &&
            !std::isdigit(static_cast<unsigned char>(text[b])) &&
            IsExpressionFunction(std::string_view(text).substr(b, e - b)) &&
            next < n && text[next] != '(' && text[next] != ')')
        {
          r.append(text, b, e - b);
          r += '(';
          i = e;
        }
        else
        {
          r += c;
          ++i;
        }
      }
      else
      {
        r += c;
        ++i;
      }
    }

    return r;
  }

  // Search path that opens the files through the game's filesystem, which
  // is where the game's menu parser reads the menus and their includes from
  // on x86. So we find the files in the iwds and the mod as it does.
  //
  class GameSearchPath final : public ISearchPath
  {
  public:
    explicit GameSearchPath(const MenuCallbacks& callbacks)
      : callbacks_(callbacks)
    {
    }

    SearchPathOpenFile Open(const std::string& fileName) override
    {
      auto data(callbacks_.read(fileName));

      if (!data)
      {
        return {};
      }

      auto text(MoveFunctionParentheses(*data));

      const auto length(static_cast<std::int64_t>(text.size()));
      return {std::make_unique<std::istringstream>(std::move(text)), length};
    }

    const std::string& GetPath() override
    {
      static const std::string path("fs_game");
      return path;
    }

    // The menu compiler only opens files by name and so we have nothing to
    // list.
    //
    void Find(const SearchPathSearchOptions&, const std::function<void(const std::string&)>&) override
    {
    }

  private:
    const MenuCallbacks& callbacks_;
  };

  // Creator that resolves an asset that the menus refer to through the
  // game's asset database.
  //
  template <typename Asset, typename T>
  class GameAssetCreator final : public AssetCreator<Asset>
  {
  public:
    explicit GameAssetCreator(const std::function<T*(const std::string&)>& find)
      : find_(find)
    {
    }

    AssetCreationResult CreateAsset(const std::string& assetName, AssetCreationContext& context) override
    {
      auto* asset(find_(assetName));

      if (asset == nullptr)
      {
        return AssetCreationResult::NoAction();
      }

      return AssetCreationResult::Success(
        context.AddAsset<Asset>(assetName, reinterpret_cast<typename Asset::Type*>(asset)));
    }

  private:
    const std::function<T*(const std::string&)>& find_;
  };

  // Point the items back at their menu, which the game's menu parser does
  // and OpenAssetTools leaves to the game's zone loader.
  //
  void SetItemParents(IW4::MenuList& list)
  {
    for (int i = 0; i < list.menuCount; ++i)
    {
      auto* menu(list.menus[i]);

      if (menu == nullptr || menu->items == nullptr)
      {
        continue;
      }

      for (int j = 0; j < menu->itemCount; ++j)
      {
        if (menu->items[j] != nullptr)
        {
          menu->items[j]->parent = menu;
        }
      }
    }
  }
}

namespace Components::OAT
{
  std::optional<MenuFile> LoadMenuFile(const std::string& file, const MenuCallbacks& c)
  {
    const auto* previous(callbacks);
    callbacks = &c;

    struct restore
    {
      const MenuCallbacks* previous;
      ~restore() { callbacks = previous; }
    } r {previous};

    // Note that the game's menu parser keeps the script of an event handler
    // as it is, and so a command it doesn't know only fails when the menu
    // runs it. OpenAssetTools only compiles such scripts in its permissive
    // mode.
    //
    ObjLoading::Configuration.MenuPermissiveParsing = true;

    try
    {
      auto zone(std::make_shared<Zone>(file, 0, GameId::IW4, GamePlatform::PC));
      GameSearchPath searchPath(c);

      AssetCreatorCollection creators(*zone);
      creators.AddAssetCreator(std::make_unique<GameAssetCreator<IW4::AssetMaterial, Game::Material>>(c.material));
      creators.AddAssetCreator(std::make_unique<GameAssetCreator<IW4::AssetSound, Game::snd_alias_list_t>>(c.sound));
      creators.AddAssetCreator(menu::CreateMenuListLoaderIW4(zone->Memory(), searchPath));

      IgnoredAssetLookup ignored;
      AssetCreationContext context(*zone, &creators, &ignored);

      const auto result(creators.CreateAsset(IW4::AssetMenuList::EnumEntry, file, context));

      if (!result.HasTakenAction() || result.HasFailed())
      {
        return std::nullopt;
      }

      creators.FinalizeZone(context);

      auto* info(static_cast<XAssetInfo<IW4::MenuList>*>(result.GetAssetInfo()));

      if (info == nullptr || info->Asset() == nullptr)
      {
        return std::nullopt;
      }

      SetItemParents(*info->Asset());

      return MenuFile {reinterpret_cast<Game::MenuList*>(info->Asset()), std::move(zone)};
    }
    catch (const std::exception& e)
    {
      Print(MenuMessage::error, e.what());
      return std::nullopt;
    }
  }
}

// The lines that the menu parser reads.
//
// Note that OpenAssetTools leaves the file name of the line that ends the
// file null and its lexer reads it when it reaches the end of a menu file.
// So we take the place of IParserLineStream.cpp and name a line without a
// file.
//
namespace
{
  // A UTF-8 BOM is an encoding signature and is only valid at the start of
  // a file.
  //
  constexpr std::string_view utf8_bom("\xEF\xBB\xBF", 3);

  std::shared_ptr<std::string> LineFileName(std::shared_ptr<std::string> name, const char* fallback)
  {
    return name != nullptr ? std::move(name) : std::make_shared<std::string>(fallback);
  }
}

ParserLine::ParserLine()
  : m_filename(LineFileName(nullptr, "<eof>")),
    m_line_number(0)
{
}

ParserLine::ParserLine(std::shared_ptr<std::string> filename, const int lineNumber, std::string line)
  : m_filename(LineFileName(std::move(filename), "<unknown>")),
    m_line_number(lineNumber),
    m_line(std::move(line))
{
  if (m_line_number == 1 && m_line.starts_with(utf8_bom))
  {
    m_line.erase(0, utf8_bom.size());
  }
}

bool ParserLine::IsEof() const
{
  return m_line_number <= 0;
}

// The log of OpenAssetTools, which we print through the callbacks of the
// file we are compiling.
//
// Note that its own prints to the standard streams, which the game doesn't
// show.
//
namespace con
{
  LogLevel _globalLogLevel = LogLevel::DEBUG;
  std::atomic_size_t _warningCount(0);
  std::atomic_size_t _errorCount(0);

  void init()
  {
  }

  void set_log_level(const LogLevel value)
  {
    _globalLogLevel = value;
  }

  void set_use_color(bool)
  {
  }

  void reset_counts()
  {
    _warningCount = 0;
    _errorCount = 0;
  }

  size_t warning_count()
  {
    return _warningCount;
  }

  size_t error_count()
  {
    return _errorCount;
  }

  void _debug_internal(const std::string& str)
  {
    Print(MenuMessage::debug, str);
  }

  void _info_internal(const std::string& str)
  {
    Print(MenuMessage::info, str);
  }

  void _warn_internal(const std::string& str)
  {
    Print(MenuMessage::warning, str);
  }

  void _error_internal(const std::string& str)
  {
    Print(MenuMessage::error, str);
  }
}
