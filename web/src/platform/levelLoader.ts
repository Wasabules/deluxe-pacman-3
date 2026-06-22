// Chargement des niveaux `.dp2` depuis les assets statiques.

import { decodeLevel } from '../core/level';
import type { Level } from '../core/types';

export async function fetchLevel(url: string): Promise<Level> {
  const res = await fetch(url);
  if (!res.ok) throw new Error(`Niveau introuvable : ${url} (HTTP ${res.status})`);
  return decodeLevel(await res.arrayBuffer());
}

export function levelPath(n: number): string {
  return `Levels/level${n.toString().padStart(3, '0')}.dp2`;
}

export function bonusPath(n: number): string {
  return `Levels/bonus${n.toString().padStart(3, '0')}.dp2`;
}
