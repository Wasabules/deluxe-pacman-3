# Deluxe Pacman 3

A modern, web-native rewrite of **Deluxe Pacman 2** — the Pacman-style arcade
game by **Neil Roy** (originally written in C + Allegro 5). The rewrite lives in
[`/web`](web) and runs in any browser: **TypeScript + WebGL (PixiJS)**, with the
original C source kept at the repository root as the reference.

> The gameplay is reproduced faithfully from the original, then extended with new
> tools, scoring, game modes and a lot of arcade polish.

## ✨ Features (web rewrite)

- **WebGL renderer** with a 2.5-D relief look (extruded walls, depth-sorted entities, ground shadows).
- **Animated synthwave arcade menu** (neon grid, CRT effect, chiptune vibe).
- **31 tools** — all the originals plus 6 new ones (Magnet, Panic, Phase, Bomb, Lightning, Warp), with an in-game **codex** explaining each.
- **Scoring system**: combo multiplier (×1→×5), floating points, end-of-level bonuses (perfection / speed / lives).
- **3 game modes**: Time Attack (3-min run), Survival (one life, ghosts speed up), Daily Challenge (shared seed) — plus the classic 50-level campaign with up to 4 hot-seat players.
- **Game feel**: particles, screen shake, neon speed trail, animated transitions, render interpolation (high-refresh ready).
- **Comfort**: real pause, level recap, save & resume, READY?/GO!.
- **Bilingual** (English / French), volume controls, gamepad support, **PWA** installable, and a built-in **web level editor**.
- Pure, deterministic `core/` (no DOM) covered by **48 unit tests** (Vitest).

## ▶️ Run the web game

```bash
cd web
npm install
npm run dev       # http://localhost:5173/  (editor at /editor.html)
npm run build     # type-check + static build into dist/
npm run preview   # serve the production build
npm test          # unit tests
```

**In-game**: arrows / WASD to move · CTRL dash · SPACE shoot (with the gun) ·
ESC pause · F fullscreen.

## 📁 Repository layout

| Path | What |
|---|---|
| `web/` | The web rewrite (TypeScript + PixiJS) — the active project. See [`web/README.md`](web/README.md). |
| `*.c`, `*.h`, `*.cbp` | The original Deluxe Pacman 2 / Pace 2 C source (Code::Blocks workspace), kept as reference. |
| `bin/` | Original game data (the `.pak` archive, assets). |
| `docs/` | Original manual, dev journal and notes. |

The C game is **Windows-only** and built with a MinGW-W64 toolchain via the
Code::Blocks project files (no Makefile). The web rewrite does **not** reuse the
C code — only the concept, rules and assets carry over.

## 🙏 Credits

All credit for the original **Deluxe Pacman 2**, its design and its assets goes
to **Neil Roy** — check out his games at
**https://nitehackr.github.io/games_index.html**.

Original game © 2019 Neil Roy. Web rewrite © 2026 Wasabules.

## 📜 License

[MIT](LICENSE) — for both the original game and the web rewrite. The graphics,
fonts and sounds are from Neil Roy's freely-distributed original release and are
included here for the faithful rewrite.
