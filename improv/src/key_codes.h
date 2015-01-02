
#pragma once

#include "circa/circa.h"

#if IMPROV_USE_SDL
  #include "SDL_keycode.h"

  SDL_Keycode nacl_keycode_to_sdl_key(uint32_t code);
  void sdl_key_to_symbol(SDL_Keycode sym, caValue* value);
#endif

void nacl_key_to_symbol(uint32_t code, caValue* value);

