// Hall of Fame persistant (localStorage). Une table de 5 scores par « catégorie »
// (mode + difficulté), ce qui donne un classement séparé pour chaque mode de jeu.

const NUM_SCORES = 5;
const KEY = 'dp2.hiscores.v2';

export interface ScoreEntry {
  name: string;
  score: number;
  level: number;
}

type Store = Record<string, ScoreEntry[]>;

function defaults(): ScoreEntry[] {
  return [
    { name: 'NEIL', score: 50000, level: 5 },
    { name: 'PAC', score: 40000, level: 4 },
    { name: 'GHOST', score: 30000, level: 3 },
    { name: 'BLINKY', score: 20000, level: 2 },
    { name: 'PLAYER', score: 10000, level: 1 },
  ];
}

function load(): Store {
  try {
    const raw = localStorage.getItem(KEY);
    if (raw) {
      const parsed = JSON.parse(raw) as Store;
      if (parsed && typeof parsed === 'object') return parsed;
    }
  } catch {
    /* stockage indisponible */
  }
  return {};
}

function save(store: Store): void {
  try {
    localStorage.setItem(KEY, JSON.stringify(store));
  } catch {
    /* ignore */
  }
}

/** Table triée (score décroissant) pour une catégorie (créée par défaut si absente). */
export function getTable(cat: string): ScoreEntry[] {
  const store = load();
  return store[cat] ?? defaults();
}

/** Un score entre-t-il dans la table de cette catégorie ? */
export function qualifies(score: number, cat: string): boolean {
  const table = getTable(cat);
  return table.length < NUM_SCORES || score > table[table.length - 1]!.score;
}

/** Insère un score et renvoie la table mise à jour (tronquée à NUM_SCORES). */
export function insertScore(cat: string, entry: ScoreEntry): ScoreEntry[] {
  const store = load();
  const table = store[cat] ?? defaults();
  table.push(entry);
  table.sort((a, b) => b.score - a.score);
  table.length = Math.min(table.length, NUM_SCORES);
  store[cat] = table;
  save(store);
  return table;
}
