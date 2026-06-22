# Deluxe Pacman 2 — réécriture web

Portage web-natif du jeu Deluxe Pacman 2 (à l'origine en C + Allegro 5), en
**TypeScript + WebGL (PixiJS)**, reproduisant le gameplay à l'identique puis
l'étendant (nouveaux outils, scoring/combos, modes de jeu, finitions arcade).
Cible principale : le navigateur ; le desktop via un wrapper Tauri.

Le source C original (dossier parent) reste la **référence** du comportement.

> Rebrandé **« Deluxe Pacman 3 »** dans le menu. Au-delà du portage fidèle :
> rendu 2.5D, menu arcade synthwave, **31 outils** (6 nouveaux + codex),
> multiplicateur de combo, **3 modes** (Time Attack / Survie / Défi du jour),
> game feel (particules, trail, transitions), pause & reprise, i18n EN/FR, PWA.

## Démarrer

```bash
npm install
npm run dev        # serveur de dev — jeu sur http://localhost:5173/
                   # éditeur de niveaux sur http://localhost:5173/editor.html
npm run build      # typecheck + build statique dans dist/
npm run preview    # sert le build de production
npm test           # tests unitaires (Vitest) — 48 tests sur le core et l'éditeur
```

## Commandes en jeu

Flèches / WASD / pavé num. : déplacement · CTRL : accélérer (énergie) ·
ESPACE : tirer (outil pistolet) · ÉCHAP : menu · F : plein écran ·
M : musique · N : son · manette Xbox supportée.

## Éditeur de niveaux (« Pace 2 »)

`editor.html` — outil web qui remplace l'éditeur natif d'origine : dessin des murs
au clic-glisser (auto-tiling), gomme, pills/powerpills, placement des spawns
(Pacman, 4 fantômes, pickup, téléports), remplissage automatique, validation
(mêmes règles que le C), import/export `.dp2` **et** JSON.

## Desktop (Tauri)

La configuration Tauri 2 est prête dans `src-tauri/`. Elle empaquette le build web
(`dist/`) en exécutable Windows/Linux/macOS. Prérequis : **Rust** (https://rustup.rs).

```bash
npm run tauri dev     # lance l'app desktop en mode dev
npm run tauri build   # génère les binaires/installeurs natifs
```

Sans Rust, `npm run tauri info` valide la configuration mais le build échoue.

## Rendu

Le **jeu** est rendu en **WebGL via PixiJS** (`render/pixi/`) : décor du niveau
(reconstruit une fois par niveau), entités et pills (mises à jour par frame). Le
**HUD et les écrans hors-jeu** sont dessinés sur un **canvas 2D superposé** (`#ui`),
transparent en jeu. Deux canvas empilés : `#gl` (WebGL) derrière, `#ui` (2D) devant.

Ce choix (vs Canvas 2D) apporte les performances GPU, élimine des bugs de canvas 2D
cross-navigateur (Firefox/WebRender), et ouvre la voie à la 2.5D. Le `core/` (logique)
est resté totalement inchangé lors de cette migration.

## Architecture (hexagonale)

Le `core/` est de la logique pure, sans dépendance navigateur, donc testable.
Les adapters `platform/` (rendu, audio, entrées, assets) sont remplaçables.

```
src/
  core/        logique pure et testable (zéro dépendance navigateur)
    constants.ts   constantes reprises de dp2_main.h / dp2_map.h
    types.ts       Tile, MapCoord, Level (transposés des structs C)
    level.ts       décodage/encodage du format binaire .dp2
    entities.ts    Pacman, Ghost
    game.ts        simulation d'un niveau (déplacement, pills, outils, collisions)
    ghosts.ts      déplacement + IA + états des fantômes
    pathfinding.ts flood-fill BFS (get_path / dir)
    tools.ts       enum des 31 outils + durées
  platform/    adapters navigateur (remplaçables)
    renderer.ts    Canvas 2D, résolution logique 800x600
    assets.ts      chargement/découpage des spritesheets
    audio.ts       WebAudio (sons + musique)
    input.ts       clavier + manette
    levelLoader.ts chargement des niveaux .dp2
    hiscore.ts     hall of fame (localStorage)
  render/
    pixi/        rendu du JEU en WebGL (PixiJS) : décor, entités, pills
      pixiGame.ts    orchestration (app PixiJS, scène, rendu manuel)
      pixiTextures.ts spritesheets → textures GPU découpées
      decorView.ts   décor d'un niveau (reconstruit par niveau)
      entityView.ts  sprites des entités (mis à jour par frame)
    menuScene.ts   menu d'accueil arcade animé (synthwave 2.5D, néon, CRT)
    screens.ts   écrans hors-jeu restants (HUD + hall of fame en overlay 2D)
  app/         orchestration
    loop.ts        boucle de jeu à pas de temps fixe (60 Hz)
    session.ts     machine d'états (intro/menu/jeu/hiscore, multi-joueurs)
    audioDirector.ts  mappe les événements de jeu aux sons
    main.ts        point d'entrée du jeu
  editor/      éditeur de niveaux
    editorState.ts logique pure (auto-tiling, validation)
    main.ts        UI de l'éditeur
public/        assets extraits du .pak (Graphics/ Sound/ Levels/ Fonts/)
src-tauri/     configuration du wrapper desktop Tauri 2
tests/         tests Vitest (48) sur le core, l'éditeur et la session
```

## Avancement

- [x] **Jalon 0** — socle (Vite + TS + Vitest), canvas mis à l'échelle, boucle à pas fixe.
- [x] **Jalon 1** — données + rendu de niveau : décodeur `.dp2`, extraction du `.pak`
  dans `public/`, rendu fidèle (fond tuilé, murs + ombres, pills, spawns, téléports).
  Vérifié visuellement par capture headless (`scripts/shot.mjs`).
- [x] **Jalon 2** — Pacman jouable : déplacement sur grille (virages bufferisés,
  blocage par les murs, wrap des tunnels), animation + orientation du sprite,
  manger pills/powerpills, score, énergie, mode rapide (CTRL). Logique pure dans
  `core/game.ts`. Vérifié par capture (déplacement + pills mangées).
- [x] **Jalon 3** — fantômes : pathfinding flood-fill (`core/pathfinding.ts`, testé),
  4 IA distinctes (difficulté), déplacement sur grille, états normal/scared/frozen/eyes,
  rendu composite (corps coloré + yeux directionnels), collisions (manger un fantôme
  bleu / mourir), vies, game over. Vérifié par captures (couleurs, chasse, frayeur).
- [x] **Jalon 4** — outils & pickups : apparition/disparition sur la case de spawn,
  collecte, les 25 effets (multiplicateurs ×2/×5/×7, diamants, present, gel, dynamite,
  bouclier, glu, vitesse, pistolet+balle, lettres EXTRA → vie, points fixes, skull…),
  pickups alimentaires, HUD (EXTRA + outil actif). 9 tests unitaires ; effets visuels
  vérifiés par capture. BONUS/JUMP chargent un niveau (mécanique fine au jalon 5).
- [x] **Jalon 5** — méta-jeu : machine d'états (`app/session.ts`) intro → menu →
  niveaux → game over → hall of fame ; enchaînement des 50 niveaux (fin de niveau,
  flash, +10000, wrap) ; multi-joueurs hot-seat (rotation à la mort) ; niveaux bonus
  (pas de perte de vie, +50000/+vie) ; hall of fame par difficulté (localStorage,
  saisie de nom). 5 tests ; écrans vérifiés par captures.
- [x] **Jalon 6** — finitions : audio WebAudio (27 sons + musique title/background
  selon l'écran et l'état des fantômes, pan stéréo) ; **téléports** (corrige une
  lacune du jalon 2, testés) ; plein écran (F) ; manette (Gamepad API, mapping Xbox) ;
  réglages (difficulté G, son M, musique N) ; **PWA** (manifest installable +
  service worker offline). Diagnostic headless OK, aucune erreur console.
- [x] **Jalon 7** — éditeur de niveaux web (`editor.html`) : auto-tiling des murs,
  gomme, pills/powerpills, spawns, remplissage, validation (règles du C), import/export
  `.dp2` et JSON. 9 tests unitaires ; vérifié par capture (niveau importé + rendu).
- [x] **Jalon 8** — packaging desktop : configuration Tauri 2 complète (`src-tauri/`),
  validée par `tauri info` (WebView2 + MSVC présents). Build natif Win/Linux/macOS
  via `npm run tauri build` une fois Rust installé.

**Portage terminé** : le jeu est complet et fidèle au Deluxe Pacman 2 original,
jouable dans le navigateur (PWA installable) et empaquetable pour le bureau.
