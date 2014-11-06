
#include "improv_common.h"

#include <stdint.h>
#include "key_codes.h"

#if IMPROV_USE_SDL

void sdl_key_to_symbol(SDL_Keycode sym, caValue* value)
{
    switch (sym) {
    case SDLK_a: return circa_set_symbol(value, "a");
    case SDLK_b: return circa_set_symbol(value, "b");
    case SDLK_c: return circa_set_symbol(value, "c");
    case SDLK_d: return circa_set_symbol(value, "d");
    case SDLK_e: return circa_set_symbol(value, "e");
    case SDLK_f: return circa_set_symbol(value, "f");
    case SDLK_g: return circa_set_symbol(value, "g");
    case SDLK_h: return circa_set_symbol(value, "h");
    case SDLK_i: return circa_set_symbol(value, "i");
    case SDLK_j: return circa_set_symbol(value, "j");
    case SDLK_k: return circa_set_symbol(value, "k");
    case SDLK_l: return circa_set_symbol(value, "l");
    case SDLK_m: return circa_set_symbol(value, "m");
    case SDLK_n: return circa_set_symbol(value, "n");
    case SDLK_o: return circa_set_symbol(value, "o");
    case SDLK_p: return circa_set_symbol(value, "p");
    case SDLK_q: return circa_set_symbol(value, "q");
    case SDLK_r: return circa_set_symbol(value, "r");
    case SDLK_s: return circa_set_symbol(value, "s");
    case SDLK_t: return circa_set_symbol(value, "t");
    case SDLK_u: return circa_set_symbol(value, "u");
    case SDLK_v: return circa_set_symbol(value, "v");
    case SDLK_w: return circa_set_symbol(value, "w");
    case SDLK_x: return circa_set_symbol(value, "x");
    case SDLK_y: return circa_set_symbol(value, "y");
    case SDLK_0: return circa_set_symbol(value, "0");
    case SDLK_1: return circa_set_symbol(value, "1");
    case SDLK_2: return circa_set_symbol(value, "2");
    case SDLK_3: return circa_set_symbol(value, "3");
    case SDLK_4: return circa_set_symbol(value, "4");
    case SDLK_5: return circa_set_symbol(value, "5");
    case SDLK_6: return circa_set_symbol(value, "6");
    case SDLK_7: return circa_set_symbol(value, "7");
    case SDLK_8: return circa_set_symbol(value, "8");
    case SDLK_9: return circa_set_symbol(value, "9");
    case SDLK_LEFT:
        return circa_set_symbol(value, "left");
    case SDLK_RIGHT:
        return circa_set_symbol(value, "right");
    case SDLK_UP:
        return circa_set_symbol(value, "up");
    case SDLK_DOWN:
        return circa_set_symbol(value, "down");
    case SDLK_SPACE:
        return circa_set_symbol(value, "space");
    default:
        return circa_set_symbol(value, "unknown");
    }
}

#endif

void nacl_key_to_symbol(uint32_t code, caValue* value)
{
    switch (code) {
    case 'A': return circa_set_symbol(value, "a");
    case 'B': return circa_set_symbol(value, "b");
    case 'C': return circa_set_symbol(value, "c");
    case 'D': return circa_set_symbol(value, "d");
    case 'E': return circa_set_symbol(value, "e");
    case 'F': return circa_set_symbol(value, "f");
    case 'G': return circa_set_symbol(value, "g");
    case 'H': return circa_set_symbol(value, "h");
    case 'I': return circa_set_symbol(value, "i");
    case 'J': return circa_set_symbol(value, "j");
    case 'K': return circa_set_symbol(value, "k");
    case 'L': return circa_set_symbol(value, "l");
    case 'M': return circa_set_symbol(value, "m");
    case 'N': return circa_set_symbol(value, "n");
    case 'O': return circa_set_symbol(value, "o");
    case 'P': return circa_set_symbol(value, "p");
    case 'Q': return circa_set_symbol(value, "q");
    case 'R': return circa_set_symbol(value, "r");
    case 'S': return circa_set_symbol(value, "s");
    case 'T': return circa_set_symbol(value, "t");
    case 'U': return circa_set_symbol(value, "u");
    case 'V': return circa_set_symbol(value, "v");
    case 'W': return circa_set_symbol(value, "w");
    case 'X': return circa_set_symbol(value, "x");
    case 'Y': return circa_set_symbol(value, "y");
    case 'Z': return circa_set_symbol(value, "z");
    case '0': return circa_set_symbol(value, "0");
    case '1': return circa_set_symbol(value, "1");
    case '2': return circa_set_symbol(value, "2");
    case '3': return circa_set_symbol(value, "3");
    case '4': return circa_set_symbol(value, "4");
    case '5': return circa_set_symbol(value, "5");
    case '6': return circa_set_symbol(value, "6");
    case '7': return circa_set_symbol(value, "7");
    case '8': return circa_set_symbol(value, "8");
    case '9': return circa_set_symbol(value, "9");

    case 32:
        return circa_set_symbol(value, "space");
    case 37:
        return circa_set_symbol(value, "left");
    case 38:
        return circa_set_symbol(value, "up");
    case 39:
        return circa_set_symbol(value, "right");
    case 40:
        return circa_set_symbol(value, "down");
    default:
        return circa_set_symbol(value, "unknown");
    }

#if 0
    if (code >= 'A' && code <= 'Z')
      return (code - 'A' + SDLK_a);
    if (code >= SDLK_0 && code <= SDLK_9)
      return code;
    const uint32_t f1_code = 112;
    if (code >= f1_code && code < f1_code + 12)
      return (code - f1_code + SDLK_F1);
    const uint32_t kp0_code = 96;
    if (code >= kp0_code && code < kp0_code + 10)
      return (code - kp0_code + SDLK_KP_0);
    switch (code) {
      case SDLK_BACKSPACE:
        return SDLK_BACKSPACE;
      case SDLK_TAB:
        return SDLK_TAB;
      case SDLK_RETURN:
        return SDLK_RETURN;
      case SDLK_PAUSE:
        return SDLK_PAUSE;
      case SDLK_ESCAPE:
        return SDLK_ESCAPE;
      case 16:
        return SDLK_LSHIFT;
      case 17:
        return SDLK_LCTRL;
      case 18:
        return SDLK_LALT;
      case 32:
        return SDLK_SPACE;
      case 37:
        return SDLK_LEFT;
      case 38:
        return SDLK_UP;
      case 39:
        return SDLK_RIGHT;
      case 40:
        return SDLK_DOWN;
      case 106:
        return SDLK_KP_MULTIPLY;
      case 107:
        return SDLK_KP_PLUS;
      case 109:
        return SDLK_KP_MINUS;
      case 110:
        return SDLK_KP_PERIOD;
      case 111:
        return SDLK_KP_DIVIDE;
      case 45:
        return SDLK_INSERT;
      case 46:
        return SDLK_DELETE;
      case 36:
        return SDLK_HOME;
      case 35:
        return SDLK_END;
      case 33:
        return SDLK_PAGEUP;
      case 34:
        return SDLK_PAGEDOWN;
      case 189:
        return SDLK_MINUS;
      case 187:
        return SDLK_EQUALS;
      case 219:
        return SDLK_LEFTBRACKET;
      case 221:
        return SDLK_RIGHTBRACKET;
      case 186:
        return SDLK_SEMICOLON;
      case 222:
        return SDLK_QUOTE;
      case 220:
        return SDLK_BACKSLASH;
      case 188:
        return SDLK_COMMA;
      case 190:
        return SDLK_PERIOD;
      case 191:
        return SDLK_SLASH;
      case 192:
        return SDLK_BACKQUOTE;
      default:
        return SDLK_UNKNOWN;
    }
#endif
}
