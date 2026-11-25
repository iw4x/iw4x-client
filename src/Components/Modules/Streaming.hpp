#pragma once

namespace Components
{
  // Texture streaming for the images that IW4 loads from .iwi files.
  //
  // IW4 reads each of these images in full on the database thread when it
  // loads a map: while loading the zone (see Load_Texture()) and, for the
  // delayed images, at the end of it (see DB_LoadDelayedImages()). The
  // texture then stays in video memory for as long as the map is loaded,
  // even if nothing ever draws with it. With the stock images this results
  // in long load times and with high resolution packs it also exhausts video
  // memory.
  //
  // Instead, we load each image at its coarsest tier, that is, only its
  // smallest mipmaps, which is normally about a sixtieth of the file. A
  // worker thread then reads the sharper tiers of the images that are
  // actually drawn. A tier is one of the picmip levels that every .iwi file
  // is laid out for (see Streaming.cpp for details).
  //
  // The images that the renderer binds are in use and are upgraded to their
  // sharpest tier as soon as the budget permits. If it doesn't, then the
  // least recently drawn images are downgraded back to their coarsest tier
  // to make room. If there is room to spare, then the idle images are
  // upgraded as well. As a result, a map that fits the budget should end up
  // the same as IW4 would have loaded it, only sooner.
  //
  // Note that IW4's picmip dvars still determine how sharp an image may get
  // (r_picmip and friends, applied with r_applyPicmip). Streaming only moves
  // an image between that tier and its coarsest.
  //
  // If r_streamTextures is disabled, then the newly loaded images are left
  // to IW4 and the streamed ones are upgraded to their sharpest tier. The
  // r_streamBudget dvar is the video memory budget in MB, with 0 meaning
  // half of what the device reports. The r_streamUploadLimit dvar is the
  // number of KB to upload per frame, which determines how fast images are
  // upgraded. The r_streamHoldTime dvar is the number of milliseconds an
  // image remains in use after it was last drawn.
  //
  // For debugging, r_streamShowUnloaded draws the images that are not yet at
  // their sharpest tier as black and the r_streamInfo command prints the
  // streaming state.
  //
  class streaming: public Component
  {
  public:
    streaming ();
  };
}
