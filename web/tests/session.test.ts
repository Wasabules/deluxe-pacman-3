import { describe, it, expect, vi } from 'vitest';
import { Session } from '../src/app/session';
import type { Input } from '../src/platform/input';
import { MAPX, MAPY, MAP_ID, MAP_VER } from '../src/core/constants';
import type { Level, Tile } from '../src/core/types';

// Niveau minimal : une case avec une seule pill, le reste vide.
function tinyLevel(): Level {
  const map: Tile[][] = [];
  for (let y = 0; y < MAPY; y++) {
    const row: Tile[] = [];
    for (let x = 0; x < MAPX; x++) row.push({ tile: 0, isPill: false, isPowerpill: false, isProtected: false });
    map.push(row);
  }
  return {
    mapId: MAP_ID,
    mapVer: MAP_VER,
    validated: true,
    lineSet: 0,
    player: { x: 11, y: 8 },
    ghosts: [
      { x: 1, y: 1 },
      { x: 21, y: 1 },
      { x: 1, y: 15 },
      { x: 21, y: 15 },
    ],
    pickup: { x: 11, y: 4 },
    teleport: [
      { x: 99, y: 99 },
      { x: 99, y: 99 },
    ],
    background: 0,
    map,
    pills: 5,
  };
}

function fakeInput(keys: string[] = []): Input {
  return {
    desiredDir: -1,
    fast: false,
    firing: false,
    takeKey: () => keys.shift() ?? null,
    clearKeys: () => {},
  } as unknown as Input;
}

const flush = () => new Promise((r) => setTimeout(r, 0));

function makeSession(): Session {
  return new Session({ loadLevel: async () => tinyLevel(), maxLevels: 50, maxBonus: 4, difficulty: 2 });
}

describe('Session — flux d\'écrans', () => {
  // Le menu (intro/menu) est désormais piloté par l'app via toMenu()/startGame() ;
  // la session expose ces transitions, le reste du flux passe par tick().
  it('intro → menu → démarrage → niveau', async () => {
    const s = makeSession();
    expect(s.screen).toBe('intro');
    s.toMenu();
    expect(s.screen).toBe('menu');
    s.startGame(1);
    expect(s.screen).toBe('loading');
    await flush();
    expect(s.screen).toBe('levelIntro');
    expect(s.play).not.toBeNull();
    expect(s.players).toHaveLength(1);
  });

  it('startGame crée le bon nombre de joueurs', async () => {
    const s = makeSession();
    s.toMenu();
    s.startGame(3);
    await flush();
    expect(s.players).toHaveLength(3);
  });

  async function reachPlaying(s: Session): Promise<void> {
    s.toMenu();
    s.startGame(1);
    await flush();
    s.tick(fakeInput(['enter'])); // levelIntro → playing (touche = skip)
    expect(s.screen).toBe('playing');
  }

  it('fin de niveau : flash puis niveau suivant avec +10000', async () => {
    const s = makeSession();
    await reachPlaying(s);
    const before = s.play!.score;
    s.play!.levelComplete = true;
    s.tick(fakeInput()); // détecte la fin → levelFlash
    expect(s.screen).toBe('levelFlash');
    for (let i = 0; i < 120; i++) s.tick(fakeInput()); // flash → écran de récap
    expect(s.screen).toBe('levelClear');
    s.tick(fakeInput(['enter'])); // touche : passe le récap → loading
    expect(s.screen).toBe('loading');
    await flush();
    expect(s.players[0]!.clevel).toBe(2);
    // Base 10000 + bonus de fin (perfection/rapidité/vies).
    expect(s.play!.score).toBeGreaterThanOrEqual(before + 10000);
  });

  it('game over (solo) → hall of fame', async () => {
    const s = makeSession();
    await reachPlaying(s);
    s.play!.score = 999999;
    s.play!.lives = -1;
    s.play!.gameOver = true;
    s.tick(fakeInput()); // détecte → gameOver
    expect(s.screen).toBe('gameOver');
    for (let i = 0; i < 160; i++) s.tick(fakeInput()); // timer game over
    expect(s.screen).toBe('hiscoreEntry'); // 999999 qualifie
  });

  it('multijoueur : à la mort, la main passe au joueur 2 avec son niveau chargé', async () => {
    const s = makeSession();
    s.toMenu();
    s.startGame(2);
    await flush();
    s.tick(fakeInput(['enter'])); // J1 : levelIntro → playing
    expect(s.screen).toBe('playing');
    expect(s.current).toBe(0);

    // J1 meurt en gardant des vies → c'est au tour du joueur 2.
    s.play!.awaitingRespawn = true;
    s.tick(fakeInput());
    expect(s.current).toBe(1);
    // Le niveau du J2 n'avait jamais été chargé : il doit l'être ici (sinon
    // s.play est null → écran noir, le bug corrigé).
    expect(s.screen).toBe('loading');
    await flush();
    expect(s.screen).toBe('levelIntro');
    expect(s.play).not.toBeNull();
  });

  it('multijoueur : élimination (game over) → joueur 2 prend la main', async () => {
    const s = makeSession();
    s.toMenu();
    s.startGame(2);
    await flush();
    s.tick(fakeInput(['enter']));
    expect(s.screen).toBe('playing');

    s.play!.lives = -1;
    s.play!.gameOver = true;
    s.tick(fakeInput());
    expect(s.players[0]!.eliminated).toBe(true);
    expect(s.current).toBe(1);
    await flush();
    expect(s.screen).toBe('levelIntro');
    expect(s.play).not.toBeNull();
  });

  it('sauvegarde la progression et permet de reprendre (Continuer)', async () => {
    const store: Record<string, string> = {};
    vi.stubGlobal('localStorage', {
      getItem: (k: string) => store[k] ?? null,
      setItem: (k: string, v: string) => void (store[k] = v),
      removeItem: (k: string) => void delete store[k],
    });
    try {
      const s = makeSession();
      s.toMenu();
      s.startGame(1);
      await flush();
      expect(s.hasSavedGame()).toBe(true);
      s.tick(fakeInput(['enter'])); // levelIntro → playing

      // Termine le niveau 1 → niveau 2 (nouvelle sauvegarde au début du niveau 2).
      s.play!.levelComplete = true;
      s.tick(fakeInput());
      for (let i = 0; i < 120; i++) s.tick(fakeInput());
      s.tick(fakeInput(['enter']));
      await flush();
      expect(s.players[0]!.clevel).toBe(2);
      const scoreAtL2 = s.play!.score;

      // Retour menu puis reprise : on retrouve niveau 2 et le score.
      s.toMenu();
      s.continueGame();
      await flush();
      expect(s.players[0]!.clevel).toBe(2);
      expect(s.play!.score).toBe(scoreAtL2);

      // Game over efface la sauvegarde.
      s.tick(fakeInput(['enter'])); // levelIntro → playing
      s.play!.lives = -1;
      s.play!.gameOver = true;
      s.tick(fakeInput());
      expect(s.hasSavedGame()).toBe(false);
    } finally {
      vi.unstubAllGlobals();
    }
  });

  it('Time Attack : le chrono s\'écoule en jouant', async () => {
    const s = makeSession();
    s.toMenu();
    s.startGame(1, 'timeattack');
    await flush();
    expect(s.timeLeft).toBeGreaterThan(0);
    s.tick(fakeInput(['enter'])); // levelIntro → playing
    const t0 = s.timeLeft;
    s.tick(fakeInput());
    expect(s.timeLeft).toBe(t0 - 1);
  });

  it('Survie : démarre avec une seule vie', async () => {
    const s = makeSession();
    s.toMenu();
    s.startGame(1, 'survival');
    await flush();
    expect(s.play!.lives).toBe(0); // 0 vie « en réserve » = une seule vie
    expect(s.play!.ghostSpeed).toBe(1); // ×1 à la première vague
  });

  it('pré-charge le niveau suivant (clevel+1) en arrière-plan', async () => {
    const calls: number[] = [];
    const s = new Session({
      loadLevel: async (clevel, bonus) => {
        calls.push(bonus > 0 ? -bonus : clevel);
        return tinyLevel();
      },
      maxLevels: 50,
      maxBonus: 4,
      difficulty: 2,
    });
    s.toMenu();
    s.startGame(1);
    await flush(); // niveau 1 chargé → déclenche le prefetch du niveau 2
    expect(calls).toContain(1); // niveau courant
    expect(calls).toContain(2); // niveau suivant pré-chargé
  });

  it('niveau bonus : mort sans perte de vie, retour au niveau normal', async () => {
    const s = makeSession();
    await reachPlaying(s);
    // Simule un outil BONUS.
    s.play!.requestJump = -1;
    s.tick(fakeInput());
    await flush();
    expect(s.players[0]!.bonusLevel).toBeGreaterThan(0);
    expect(s.screen).toBe('levelIntro');
  });
});
