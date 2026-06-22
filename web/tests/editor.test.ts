import { describe, it, expect } from 'vitest';
import {
  initEmptyLevel,
  placeLine,
  removeTile,
  placeSpawn,
  pillFill,
  syncPills,
  validate,
} from '../src/editor/editorState';
import { encodeLevel, decodeLevel } from '../src/core/level';

describe('éditeur — auto-tiling des murs', () => {
  it('un point isolé a la tuile 1 (aucune connexion)', () => {
    const lv = initEmptyLevel();
    placeLine(lv, 5, 5, -1, -1);
    expect(lv.map[5][5].tile).toBe(1); // bits=0
  });

  it('connecter deux cases pose les bons bits de chaque côté', () => {
    const lv = initEmptyLevel();
    placeLine(lv, 5, 5, -1, -1);
    placeLine(lv, 6, 5, 5, 5); // déplacement vers la droite
    // (6,5) connecte à gauche → bit 8 → tuile 9
    expect(lv.map[5][6].tile).toBe(9);
    // (5,5) connecte à droite → bit 2 → tuile 3
    expect(lv.map[5][5].tile).toBe(3);
  });

  it('effacer une case sévère la connexion du voisin', () => {
    const lv = initEmptyLevel();
    placeLine(lv, 5, 5, -1, -1);
    placeLine(lv, 6, 5, 5, 5);
    removeTile(lv, 6, 5);
    expect(lv.map[5][6].tile).toBe(0); // effacée
    expect(lv.map[5][5].tile).toBe(1); // a perdu son bit droite → point isolé
  });

  it('ne dessine pas sur une case protégée (spawn)', () => {
    const lv = initEmptyLevel();
    placeSpawn(lv, 'player', 5, 5);
    placeLine(lv, 5, 5, -1, -1);
    expect(lv.map[5][5].tile).toBe(0); // inchangé
    expect(lv.player).toEqual({ x: 5, y: 5 });
  });
});

describe('éditeur — validation', () => {
  function validLevel() {
    const lv = initEmptyLevel();
    placeSpawn(lv, 'player', 11, 8);
    placeSpawn(lv, 'ghost0', 1, 1);
    placeSpawn(lv, 'ghost1', 21, 1);
    placeSpawn(lv, 'ghost2', 1, 15);
    placeSpawn(lv, 'ghost3', 21, 15);
    placeSpawn(lv, 'pickup', 11, 4);
    pillFill(lv, 0);
    syncPills(lv);
    return lv;
  }

  it('rejette un niveau sans Pacman', () => {
    const lv = initEmptyLevel();
    const r = validate(lv);
    expect(r.valid).toBe(false);
    expect(r.message).toMatch(/Pacman/);
  });

  it('rejette un fantôme manquant', () => {
    const lv = initEmptyLevel();
    placeSpawn(lv, 'player', 11, 8);
    placeSpawn(lv, 'pickup', 11, 4); // le pickup est validé avant les fantômes
    pillFill(lv, 0);
    const r = validate(lv);
    expect(r.valid).toBe(false);
    expect(r.message).toMatch(/Fantôme/);
  });

  it('accepte un niveau ouvert complet', () => {
    const r = validate(validLevel());
    expect(r.message).toBe('Niveau valide ✓');
    expect(r.valid).toBe(true);
  });

  it('rejette un seul téléport (il en faut 0 ou 2)', () => {
    const lv = validLevel();
    placeSpawn(lv, 'teleport', 0, 8);
    const r = validate(lv);
    expect(r.valid).toBe(false);
    expect(r.message).toMatch(/[Tt]éléport/);
  });
});

describe('éditeur — export/import .dp2', () => {
  it('round-trip d\'un niveau créé dans l\'éditeur', () => {
    const lv = initEmptyLevel();
    placeSpawn(lv, 'player', 11, 8);
    placeLine(lv, 3, 3, -1, -1);
    placeLine(lv, 4, 3, 3, 3);
    pillFill(lv, 0);
    syncPills(lv);
    const decoded = decodeLevel(encodeLevel(lv));
    expect(decoded.map[3][4].tile).toBe(lv.map[3][4].tile);
    expect(decoded.player).toEqual(lv.player);
    expect(decoded.pills).toBe(lv.pills);
  });
});
