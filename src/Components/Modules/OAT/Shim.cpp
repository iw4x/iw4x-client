#include "Shim.hpp"

#include <algorithm>
#include <atomic>
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <format>
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
#include <Game/IW4/ObjConstantsIW4.h>
#include <Game/IW4/Weapon/InfoStringLoaderWeaponIW4.h>
#include <Game/IW4/Weapon/WeaponFields.h>
#include <InfoString/InfoString.h>
#include <ObjLoading.h>
#include <Parsing/IParserLineStream.h>
#include <Pool/XAssetInfo.h>
#include <SearchPath/ISearchPath.h>
#include <Utils/Logging/Log.h>
#include <Zone/Zone.h>

// Check that OAT lays the menu structures out the way Shim.hpp says. This
// is our half of the check, and Menus.cpp does the game's (see Shim.hpp for
// why it takes two).
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

  static_assert(sizeof(IW4::WeaponCompleteDef) == WeaponCompleteDef);
  static_assert(sizeof(IW4::WeaponDef) == WeaponDef);
}

namespace
{
  using namespace Components::OAT;

  using PrintFunction = std::function<void (Message, const std::string&)>;

  // The print callback of the file being compiled.
  //
  // OAT prints through free functions (see con below), which have no way to
  // know whose file they are printing about, so we keep the callback here
  // for the duration of LoadMenuFile() or LoadWeaponFile(). It's
  // thread-local so that two threads compiling at once wouldn't print into
  // each other's callbacks, though as far as we know the menus and weapons
  // are only ever loaded from the main thread.
  //
  thread_local const PrintFunction* print = nullptr;

  void Print(const Message message, const std::string& text)
  {
    if (print != nullptr && *print)
    {
      (*print)(message, text);
    }
  }

  // Set the print callback for as long as we are in scope.
  //
  class PrintScope
  {
  public:
    explicit PrintScope(const PrintFunction& f)
      : previous_(print)
    {
      print = &f;
    }

    ~PrintScope()
    {
      print = previous_;
    }

    PrintScope(const PrintScope&) = delete;
    PrintScope& operator=(const PrintScope&) = delete;

  private:
    const PrintFunction* previous_;
  };

  // Return true if the name is that of a function of the menu expressions,
  // like sin or milliseconds, ignoring case the same as the game.
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

  // Rewrite the function calls of the menu expressions the way OAT expects
  // them.
  //
  // The menus that we dumped from the game's zones write the parenthesis
  // that opens a function's argument before the function's name, as in
  // ( sin milliseconds( ) / 1500 ) for what one would write as
  // sin( milliseconds( ) / 1500 ). That's presumably how the dumper prints
  // the compiled expression, and the game's parser is fine with it, since
  // it takes a function name right after a parenthesis as opening that
  // function. OAT isn't, it wants the parenthesis after the name. And many
  // of our menus started out as such dumps, so they are full of this.
  //
  // We could fix the menus themselves, of course, but then every mod's
  // menus that were dumped the same way would still break. So instead we
  // move such a parenthesis after the name as we read the file, which means
  // the same thing to the game anyway. We only do it when the parenthesis is
  // followed by a known function name and the name isn't already followed
  // by a parenthesis of its own, and we leave strings and comments alone,
  // which should keep us from rewriting anything that was fine to begin
  // with.
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

  // Open the files through the game's file system, which is where the
  // game's own parser reads the menus and their includes from on x86. That
  // way we find them in the iwds and the mod the same as it does.
  //
  using ReadFunction = std::function<std::optional<std::string> (const std::string&)>;

  class GameSearchPath final : public ISearchPath
  {
  public:
    explicit GameSearchPath(const ReadFunction& read)
      : read_(read)
    {
    }

    SearchPathOpenFile Open(const std::string& fileName) override
    {
      auto data(read_(fileName));

      if (!data)
      {
        return {};
      }

      // Every file goes through the rewrite, includes too, since presumably
      // the dumped .inc files have the same kind of expressions in them.
      //
      auto text(MoveFunctionParentheses(*data));

      const auto length(static_cast<std::int64_t>(text.size()));
      return {std::make_unique<std::istringstream>(std::move(text)), length};
    }

    const std::string& GetPath() override
    {
      static const std::string path("fs_game");
      return path;
    }

    // The menu compiler only ever opens files by name, so there is nothing for
    // us to list here.
    //
    void Find(const SearchPathSearchOptions&, const std::function<void(const std::string&)>&) override
    {
    }

  private:
    const ReadFunction& read_;
  };

  // Resolve an asset that a menu or a weapon refers to, a material or a
  // sound, say, through the game's asset database rather than having OAT
  // load it from disk.
  //
  // If the game doesn't have it, then we take no action and leave it to OAT,
  // which presumably reports it as missing.
  //
  template <typename Asset>
  class GameAssetCreator final : public AssetCreator<Asset>
  {
  public:
    explicit GameAssetCreator(std::function<void* (const std::string&)> find)
      : find_(std::move(find))
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
    std::function<void* (const std::string&)> find_;
  };

  // Point each item back at its menu. The game's parser does that as it
  // parses, while OAT leaves it to the game's zone loader, which fills it in
  // when it loads a menu from a zone. Our menus never go through that, so
  // without this every item would be left without a parent.
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

  template <typename Asset>
  void AddGameAssetCreator(AssetCreatorCollection& creators, const WeaponCallbacks& c)
  {
    creators.AddAssetCreator(std::make_unique<GameAssetCreator<Asset>>([&c](const std::string& n)
    {
      return c.find(Asset::EnumEntry, n);
    }));
  }

  // Turn the sound names of the weapon into the game's sounds.
  //
  // OAT leaves the names in, the same as a zone has them, and it's the
  // game's zone loader that looks them up as it loads the weapon. Note
  // that the field is a union of the two, so we read the name before we
  // overwrite it.
  //
  void ResolveSounds(IW4::WeaponFullDef& weapon, const WeaponCallbacks& c)
  {
    const auto resolve([&c](IW4::SndAliasCustom& s)
    {
      if (s.name != nullptr && s.name->soundName != nullptr)
      {
        const std::string name(s.name->soundName);
        s.sound = static_cast<IW4::snd_alias_list_t*>(c.find(IW4::ASSET_TYPE_SOUND, name));
      }
    });

    // The offsets of the fields are into the full def.
    //
    auto* base(reinterpret_cast<std::byte*>(&weapon));

    for (const auto& f : IW4::weapon_fields)
    {
      if (f.iFieldType == IW4::CSPFT_SOUND)
      {
        resolve(*reinterpret_cast<IW4::SndAliasCustom*>(base + f.iOffset));
      }
    }

    // The bounce sounds aren't a field of their own but an array of one
    // sound for each surface type.
    //
    if (weapon.weapDef.bounceSound != nullptr)
    {
      for (int i(0); i < IW4::SURF_TYPE_NUM; ++i)
      {
        resolve(weapon.weapDef.bounceSound[i]);
      }
    }
  }

  // Give each string of the weapon that the file doesn't set the empty
  // string.
  //
  // OAT only converts the keys that the file has, so the strings of the
  // others stay null. The game's parser, on the other hand, gets the empty
  // string for a missing key, and the game relies on that: it sets up a
  // weapon's xanims by reading the first character of every name, so a
  // file without, say, a sprint animation would crash it. The fastfiles
  // have empty strings there as well.
  //
  void FillEmptyStrings(IW4::WeaponFullDef& weapon)
  {
    auto* base(reinterpret_cast<std::byte*>(&weapon));

    for (const auto& f : IW4::weapon_fields)
    {
      if (f.iFieldType == IW4::CSPFT_STRING || f.iFieldType == IW4::WFT_ANIM_NAME)
      {
        auto& s(*reinterpret_cast<const char**>(base + f.iOffset));

        if (s == nullptr)
        {
          s = "";
        }
      }
    }

    // The fields cover the anims that a file can set, which may not be all
    // of the arrays, so we go over those as a whole as well.
    //
    for (auto* anims : {weapon.szXAnims, weapon.szXAnimsRightHanded, weapon.szXAnimsLeftHanded})
    {
      for (int i(0); i < IW4::NUM_WEAP_ANIMS; ++i)
      {
        if (anims[i] == nullptr)
        {
          anims[i] = "";
        }
      }
    }
  }

  // Turn the script strings of the weapon, the hide tags and the notetrack
  // maps, into the game's.
  //
  // OAT numbers them by its zone's own list of script strings, which the
  // game's zone loader would translate as it loads the zone. We translate
  // them here instead, with OAT's null string (or an empty one) as the
  // game's, 0.
  //
  void ResolveScriptStrings(IW4::WeaponFullDef& weapon, const ZoneScriptStrings& strings, const WeaponCallbacks& c)
  {
    const auto resolve([&strings, &c](std::uint16_t* s, const int count)
    {
      for (int i(0); s != nullptr && i < count; ++i)
      {
        bool null(false);
        const auto& text(s[i] < strings.Count() ? strings.Value(s[i], null) : std::string());

        s[i] = null || text.empty() ? 0 : c.string(text.c_str());
      }
    });

    resolve(weapon.weapCompleteDef.hideTags, 32);
    resolve(weapon.weapDef.notetrackSoundMapKeys, 16);
    resolve(weapon.weapDef.notetrackSoundMapValues, 16);
    resolve(weapon.weapDef.notetrackRumbleMapKeys, 16);
    resolve(weapon.weapDef.notetrackRumbleMapValues, 16);
  }
}

namespace Components::OAT
{
  // The way OAT works is that it compiles assets into a zone, the same as if
  // it was building a fast file, with a collection of creators that each
  // know how to make one kind of asset. So we make a zone of our own for
  // the file and ask it for the file as a menu list, with OAT's menu list
  // creator doing the work and ours supplying the assets the menus refer
  // to. The compiled menus then live in the zone's memory, which is what we
  // hand back as the keepalive.
  //
  std::optional<MenuFile> LoadMenuFile(const std::string& file, const MenuCallbacks& c)
  {
    PrintScope scope(c.print);

    // The game's parser keeps the script of an event handler as it is, so a
    // command it doesn't know only fails once the menu runs it. OAT, on the
    // other hand, compiles the scripts and rejects such a command right away,
    // unless it's in its permissive mode. Our menus are written for the game,
    // not for OAT, so permissive it is.
    //
    ObjLoading::Configuration.MenuPermissiveParsing = true;

    try
    {
      auto zone(std::make_shared<Zone>(file, 0, GameId::IW4, GamePlatform::PC));
      GameSearchPath searchPath(c.read);

      AssetCreatorCollection creators(*zone);
      creators.AddAssetCreator(std::make_unique<GameAssetCreator<IW4::AssetMaterial>>([&c](const std::string& n) -> void* { return c.material(n); }));
      creators.AddAssetCreator(std::make_unique<GameAssetCreator<IW4::AssetSound>>([&c](const std::string& n) -> void* { return c.sound(n); }));
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
      Print(Message::error, e.what());
      return std::nullopt;
    }
  }

  // As with a menu file, we compile the weapon into a zone of our own. OAT's
  // raw weapon loader would open the file through the search path, but it
  // also insists on the file name it makes up from the weapon's name, so we
  // read the file ourselves and hand the info string to the loader
  // underneath it.
  //
  std::optional<WeaponFile> LoadWeaponFile(const std::string& file, const std::string& name, const WeaponCallbacks& c)
  {
    PrintScope scope(c.print);

    try
    {
      auto data(c.read(file));

      if (!data)
      {
        return std::nullopt;
      }

      // A few of the raw files out there (some of ours included) end with a
      // stray separator after the last value. The game's parser doesn't mind,
      // but OAT's takes it for the start of an empty key and gives up on the
      // file. So we drop it, along with any line break after it.
      //
      while (!data->empty() && (data->back() == '\n' || data->back() == '\r'))
      {
        data->pop_back();
      }

      if (!data->empty() && data->back() == '\\')
      {
        data->pop_back();
      }

      std::istringstream is(std::move(*data));
      InfoString info;

      if (!info.FromStream(IW4::INFO_STRING_PREFIX_WEAPON, is))
      {
        Print(Message::error, std::format("could not parse {} as a weapon file", file));
        return std::nullopt;
      }

      auto zone(std::make_shared<Zone>(name, 0, GameId::IW4, GamePlatform::PC));
      GameSearchPath searchPath(c.read);

      // Everything that the weapon refers to comes from the game, the same
      // as for the game's own parser. Note that the xanims are only
      // referred to by name, so all we would get out of a missing one is a
      // warning.
      //
      AssetCreatorCollection creators(*zone);
      AddGameAssetCreator<IW4::AssetXModel>(creators, c);
      AddGameAssetCreator<IW4::AssetMaterial>(creators, c);
      AddGameAssetCreator<IW4::AssetFx>(creators, c);
      AddGameAssetCreator<IW4::AssetTracer>(creators, c);
      AddGameAssetCreator<IW4::AssetPhysCollMap>(creators, c);
      AddGameAssetCreator<IW4::AssetSound>(creators, c);
      AddGameAssetCreator<IW4::AssetXAnim>(creators, c);

      IgnoredAssetLookup ignored;
      AssetCreationContext context(*zone, &creators, &ignored);

      weapon::InfoStringLoaderIW4 loader(zone->Memory(), searchPath, *zone);
      const auto result(loader.CreateAsset(name, info, context));

      if (!result.HasTakenAction() || result.HasFailed())
      {
        return std::nullopt;
      }

      auto* asset(static_cast<XAssetInfo<IW4::WeaponCompleteDef>*>(result.GetAssetInfo()));

      if (asset == nullptr || asset->Asset() == nullptr)
      {
        return std::nullopt;
      }

      // The loader allocates the weapon as a full def, whose complete def
      // comes first.
      //
      static_assert(offsetof(IW4::WeaponFullDef, weapCompleteDef) == 0);
      auto& weapon(*reinterpret_cast<IW4::WeaponFullDef*>(asset->Asset()));

      FillEmptyStrings(weapon);
      ResolveSounds(weapon, c);
      ResolveScriptStrings(weapon, zone->m_script_strings, c);

      return WeaponFile {reinterpret_cast<Game::WeaponCompleteDef*>(&weapon.weapCompleteDef), std::move(zone)};
    }
    catch (const std::exception& e)
    {
      Print(Message::error, e.what());
      return std::nullopt;
    }
  }
}

namespace
{
  constexpr std::string_view utf8_bom("\xEF\xBB\xBF", 3);

  std::shared_ptr<std::string> LineFileName(std::shared_ptr<std::string> name, const char* fallback)
  {
    return name != nullptr ? std::move(name) : std::make_shared<std::string>(fallback);
  }

  bool IsNameCharacter(const char c)
  {
    return std::isalnum(static_cast<unsigned char>(c)) != 0 || c == '_';
  }

  // Rewrite an #elifdef or #elifndef line into the #elif that OAT knows.
  //
  // On x86 BotLib's precompiler, which the game parses menus with, gains
  // these two directives (they are C++23's, as it happens), so a mod's
  // menus may use them. Ours don't. OAT's precompiler doesn't have them,
  // and x64 doesn't have the game's precompiler at all, so there is
  // nothing to hook. But #elifdef X is just #elif defined(X), and
  // #elifndef X is #elif !defined(X), which OAT does understand. So we
  // rewrite the line as it's read, before any of OAT's proxies get to see
  // it.
  //
  // Note that if the name is missing, then we leave a bare #elif, which
  // OAT rejects for its missing expression, much like x86 rejects the
  // directive for its missing name.
  //
  void RewriteElifdef(std::string& line)
  {
    const auto hash(line.find_first_not_of(" \t"));
    if (hash == std::string::npos || line[hash] != '#')
    {
      return;
    }

    const auto directive(line.find_first_not_of(" \t", hash + 1));
    if (directive == std::string::npos)
    {
      return;
    }

    bool negate;
    std::size_t end;

    if (line.compare(directive, 8, "elifndef") == 0)
    {
      negate = true;
      end = directive + 8;
    }
    else if (line.compare(directive, 7, "elifdef") == 0)
    {
      negate = false;
      end = directive + 7;
    }
    else
    {
      return;
    }

    // Leave alone a longer name that merely starts with the directive.
    //
    if (end < line.size() && IsNameCharacter(line[end]))
    {
      return;
    }

    auto nameEnd(line.find_first_not_of(" \t", end));
    const auto nameStart(nameEnd);

    while (nameEnd < line.size() && IsNameCharacter(line[nameEnd]))
    {
      ++nameEnd;
    }

    std::string r(line, 0, directive);
    r += "elif";

    if (nameStart != std::string::npos && nameEnd > nameStart)
    {
      r += negate ? " !defined(" : " defined(";
      r.append(line, nameStart, nameEnd - nameStart);
      r += ')';
      r.append(line, nameEnd);
    }

    line = std::move(r);
  }
}

// The lines that OAT's parser reads.
//
// We take the place of OAT's IParserLineStream.cpp here, which we don't
// build. And while at it we change one detail: OAT leaves the file name of
// the line that ends a file null, but its lexer then reads that name when
// it reaches the end of a menu file. So rather than leave it null, we give
// such a line a name of its own.
//
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
  // A UTF-8 BOM is an encoding signature and is only valid at the start of
  // a file, so that's the only place we strip it. Some editors like to add
  // one, and we would rather not have the lexer take it for part of the
  // first token.
  //
  if (m_line_number == 1 && m_line.starts_with(utf8_bom))
  {
    m_line.erase(0, utf8_bom.size());
  }

  RewriteElifdef(m_line);
}

bool ParserLine::IsEof() const
{
  return m_line_number <= 0;
}

// OAT's log, which we print through the callbacks of the file being
// compiled.
//
// OAT's own implementation prints to the standard streams, which nobody
// ever sees in the game, so its warnings about our menus would just
// vanish. This way they end up in the console with the rest.
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
    Print(Message::debug, str);
  }

  void _info_internal(const std::string& str)
  {
    Print(Message::info, str);
  }

  void _warn_internal(const std::string& str)
  {
    Print(Message::warning, str);
  }

  void _error_internal(const std::string& str)
  {
    Print(Message::error, str);
  }
}
