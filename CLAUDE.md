# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project

Deluxe Pacman 2 — a single-player (up to 4 hot-seat players) Pacman-style game written in **pure C11** (`-std=gnu11`, not C++) on top of the **Allegro 5** library. By Neil Roy, released under MIT (`docs/LICENSE.txt`). Windows-only (Win32 platform set in the project files).

The repository is a **Code::Blocks workspace** (`Deluxe_Pacman_2.workspace`) containing **two projects** that share several source modules:

- **`Deluxe_Pacman_2 .cbp`** — the game itself (note the space before `.cbp`). Entry point: `dp2_main.c`.
- **`Pace_2.cbp`** — "Pace 2", the standalone **level editor**. Entry point: `p2_main.c`.

Shared between both: `dp2_map.*`, `dp2_gui.*`, `dp2_config.*`, `a5_scale_screen.*`, `vec2d_stack.*`, `a5_error.*`. A change to any shared module affects both projects — rebuild both.

## Building

There is **no Makefile or CMake** in this repo. The build is driven entirely by the Code::Blocks `.cbp` project files. To build, open `Deluxe_Pacman_2.workspace` (or an individual `.cbp`) in Code::Blocks with a **MinGW-W64** toolchain and build a target.

Toolchain / flags baked into the `.cbp` files (replicate these if invoking GCC directly):

- **32-bit only**: `-m32` (game uses `-march=i686`, editor `-march=pentium4`).
- `-std=gnu11 -Werror` — **warnings are errors**; code must compile clean. `-Wno-unused` is set on debug/profile targets only.
- Statically links the **Allegro 5 monolith** (`-DALLEGRO_STATICLINK`, `-static`) plus a long list of dependency libs (dumb, FLAC, vorbis, freetype, ogg, physfs, png16, zlib, opus, jpeg, Win32 system libs). See the `<Linker>` block in either `.cbp` for the exact list.
- Build targets: `Debug` (`-Og -ggdb3 -DDEBUG`), `Profile` (adds `-pg`, game only), `Release` (`-O2 -DRELEASE -s`). Each links a different Allegro variant (`allegro_monolith-debug-static` / `-profile-static` / `-static`).
- Output goes to `bin/`; the **working directory is `bin/`** — the game expects its data files (`.pak`, `.ini`, `Custom/`, etc.) to live there at runtime.

Allegro 5 is not vendored; it must be installed/compiled separately (see `docs/Dependencies.txt`). The `.depend` files are Code::Blocks-generated and not hand-edited.

There are **no tests** (this is a game; no test framework is present).

## Runtime data model (PhysFS + the `.pak` file)

This is the most important non-obvious mechanism in the codebase.

- `bin/Deluxe Pacman 2.pak` is **a ZIP archive renamed to `.pak`**. At startup it is mounted at `/` via `PHYSFS_mount(...)` (`dp2_main.c` ~line 1083). It contains the built-in levels (`Levels/level###.dp2`, `Levels/bonus###.dp2`), graphics, fonts and sounds.
- The code constantly **switches the active file interface** between the PhysFS archive and the real disk:
  - `al_set_physfs_file_interface()` → reads come from the `.pak`.
  - `al_set_standard_file_interface()` → reads/writes go to the real `bin/` folder.
  - Disk-based assets that use the standard interface: user levels (`Custom/level###.dp2`, which **override** the matching pak level), high-score files (`Deluxe Pacman 2.hi0/1/2`), config (`Deluxe Pacman 2.ini`), and screenshots.
  - When loading custom levels the code saves/restores the interface (often via `ALLEGRO_STATE` + `disk_load` flags). **When adding any file I/O, set the correct interface and restore it afterward**, or reads will silently hit the wrong filesystem.

## Game architecture

- **`dp2_main.c` (~5000 lines)** holds `main()`, `initialize()`, the main menu (`main_menu`), and the core game loop. The author deliberately uses **module-scope global state** (`setting`, `pacman`, `player[4]`, `ghost[4]`, `level`, `tool`, `pickup`, plus all Allegro bitmaps/samples/fonts) — declared at the top of `dp2_main.c` and `extern`-ed in `dp2_main.h`. This is intentional, not accidental; don't "fix" it into a refactor unless asked.
- **Event/timer loop**: an `ALLEGRO_EVENT_QUEUE` is fed by several timers. `setting.redraw_timer` runs at `REDRAW_TIMER` (60) Hz and drives screen redraws; **separate `pacman.timer` and `ghost[0].timer`** run at their own speeds (`TIMER_SPEED`) to control movement cadence independently of the framerate, and are sped up/slowed (e.g. `>> setting.suicide`, `* 1.5` when "fast") to change game speed.
- **Fixed logical resolution `800x600`** (`WIDTH`/`HEIGHT`). `a5_scale_screen.c` computes `scale_x/scale_y/offset_x/offset_y` to letterbox/scale the 800x600 buffer to the actual display in fullscreen/window modes.
- **Levels**: the `LEVEL` struct (`dp2_map.h`) is the on-disk format — header `MAP_ID "Pace2"`, `MAP_VER 3` (ver 3 added teleports), a `MAPX(23) x MAPY(17)` grid of `TILE` structs, plus player/ghost/pickup/teleport spawn `MAP` coords. `TILE_SIZE` is 32px. Ghost pathfinding lives in `dp2_map.c` (`get_path`, `dir`) and uses `vec2d_stack.c` (a generic int-pair stack).
- **Key tunables** are `#define`s at the top of `dp2_main.h` (`MAX_LEVELS`, `MAX_LIVES`, `MAX_ENERGY`, `SPRITE_SIZE`, `MOVE_PIXELS`, the `TOOLS` enum, etc.). The `TOOLS` enum order **must match the tool sprite sheet** (left-to-right, top-to-bottom) — reordering it desyncs the graphics.
- **Difficulty** is hard-set to 3 tiers (`DIFFICULTY 3`); high-score tables are stored per difficulty in `Deluxe Pacman 2.hi0/1/2`.

### Module map

| File | Responsibility |
|---|---|
| `dp2_main.*` | game loop, globals, menus, init/shutdown, gameplay logic |
| `dp2_options.c` | in-game options screen (large, ~46k) |
| `dp2_map.*` | `LEVEL`/`TILE` format + ghost pathfinding |
| `dp2_hiscore.*` | Hall of Fame tables, Unicode name entry, `.hi#` files |
| `dp2_config.*` | `.ini` load/save (`SETTING` struct, `option()` helper) |
| `dp2_gui.*` | `BUTTON` widget (used by editor and menus) |
| `dp2_collision.*`, `dp2_sound.*`, `dp2_message.*` | as named |
| `a5_*` | small Allegro 5 helpers (error dialog, screen scaling, screenshot) |
| `vec2d_stack.*` | generic stack used by pathfinding |
| `p2_main.c`, `p2_edit.*` | level editor application + map editing ops |

## Conventions

Style is documented in the banner comment at the top of `dp2_main.c` and in `docs/Additional Notes.txt`. Match the existing code:

- **K&R braces, 3-space indentation (spaces, not tabs).** `else`/`else if` on their own line. Braces omitted for single-statement bodies.
- Identifiers: variables and functions are `lower_case_with_underscores`; macros, structs, typedefs are `UPPER_CASE_WITH_UNDERSCORES`. Filenames capitalize the first letter of each word with underscores, lowercase extension.
- Searchable markers used throughout the source:
  - `/// TODO` — author's unfinished ideas.
  - `/// *** HACK CHECK ***` — anti-cheat code (see below).

## Anti-cheat / `HACK_PROTECTION`

Anti-tamper checks are **compiled out by default** — `#define HACK_PROTECTION` in `dp2_main.h` is commented out. When enabled, the checks set `hack_detected` (which then subtly corrupts gameplay rather than alerting the user) and validate things like the `.pak` file size against `#define PAKSIZE` in `dp2_main.h`. **If you modify the `.pak`, update `PAKSIZE` to its new byte size**, or set `pakman=1` in `Deluxe Pacman 2.ini` to skip the size check. Background and rationale: `docs/Deluxe Pacman 2 - notes on hack prevention.txt` and `docs/Additional Notes.txt`.

## Further docs

`docs/` contains the player Manual, a long development Journal, dependency notes, the hack-prevention notes, and the license. These are reference material, not build inputs.
