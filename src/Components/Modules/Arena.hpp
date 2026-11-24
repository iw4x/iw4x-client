#pragma once

#include <span>

namespace Components
{
  // The arenas and the map list, without IW4's limits on the number of maps or
  // on the length of their names.
  //
  // To lift those limits we move IW4's arrays elsewhere (see Arena.cpp for
  // how) and our map list elements are newMapArena_t rather than mapInfo. So
  // whatever is left in sharedUiInfo.mapList is stale and the map list should
  // be reached through the functions below instead.
  //
  class arena: public Component
  {
  public:
    arena ();

    // Return the map list in the order of the arena file.
    //
    // Note that IW4 only reloads the arenas when asked to (see
    // Maps::ForceRefreshArenas()), so call Game::UI_UpdateArenas() first if
    // the list may be out of date.
    //
    static std::span<const Game::newMapArena_t>
    maps ();

    // Return the map at position i of the list as the menus show it (that is,
    // sorted by display name) or NULL if there is no such position. This is
    // the position that ui_currentMap holds, which is easy to mistake for an
    // index into maps() (Vote did at one point).
    //
    static const Game::newMapArena_t*
    sorted_map (int i);
  };
}
