// Lecture/écriture du format binaire de niveau `.dp2`.
//
// Format (transposé de loadmap/savemap dans p2_edit.c), octets séquentiels :
//   "Pace2"            5 octets  (MAP_ID, sans terminateur)
//   map_ver            1 octet
//   validated          1 octet
//   line_set           1 octet
//   player.x, player.y 2 octets
//   ghost[0..3].x/.y   8 octets
//   pickup.x, pickup.y 2 octets
//   teleport[0..1].x/y 4 octets  (UNIQUEMENT si map_ver >= 3)
//   background         1 octet
//   pour chaque cellule [MAPY][MAPX] :
//     tile, is_pill, is_powerpill   3 octets
//     is_protected                  1 octet (UNIQUEMENT si map_ver >= 3)
//   pills              2 octets  (uint16 little-endian)
//
// En v3 « pleine » : 5+1+1+1+2+8+2+4+1 + MAPY*MAPX*4 + 2 = 1591 octets.

import { MAP_ID, MAP_VER, MAPX, MAPY, TELEPORT_UNSET } from './constants';
import type { Level, MapCoord, Tile } from './types';

class ByteReader {
  private readonly view: DataView;
  private pos = 0;

  constructor(buf: ArrayBuffer | Uint8Array) {
    const u8 = buf instanceof Uint8Array ? buf : new Uint8Array(buf);
    this.view = new DataView(u8.buffer, u8.byteOffset, u8.byteLength);
  }

  u8(): number {
    return this.view.getUint8(this.pos++);
  }

  u16le(): number {
    const v = this.view.getUint16(this.pos, true);
    this.pos += 2;
    return v;
  }

  ascii(len: number): string {
    let s = '';
    for (let i = 0; i < len; i++) s += String.fromCharCode(this.view.getUint8(this.pos++));
    return s;
  }
}

/** Une cellule est-elle un point de spawn ? (pour dériver is_protected des vieux niveaux). */
function isSpawn(
  x: number,
  y: number,
  player: MapCoord,
  ghosts: readonly MapCoord[],
  pickup: MapCoord,
  teleport: readonly MapCoord[],
): boolean {
  if (player.x === x && player.y === y) return true;
  if (pickup.x === x && pickup.y === y) return true;
  if (ghosts.some((g) => g.x === x && g.y === y)) return true;
  if (teleport.some((t) => t.x === x && t.y === y)) return true;
  return false;
}

/** Décode un buffer `.dp2` en objet Level. Lève une erreur si le format est invalide. */
export function decodeLevel(buf: ArrayBuffer | Uint8Array): Level {
  const r = new ByteReader(buf);

  const mapId = r.ascii(MAP_ID.length);
  if (mapId !== MAP_ID) {
    throw new Error(`Niveau invalide : map id "${mapId}", attendu "${MAP_ID}".`);
  }

  const fileVer = r.u8();
  if (fileVer > MAP_VER) {
    throw new Error(`Version de niveau non supportée : ${fileVer} (max ${MAP_VER}).`);
  }

  const validated = r.u8() !== 0;
  const lineSet = r.u8();
  const player: MapCoord = { x: r.u8(), y: r.u8() };

  const ghosts = [] as MapCoord[];
  for (let i = 0; i < 4; i++) ghosts.push({ x: r.u8(), y: r.u8() });

  const pickup: MapCoord = { x: r.u8(), y: r.u8() };

  const teleport = [] as MapCoord[];
  if (fileVer >= 3) {
    for (let i = 0; i < 2; i++) teleport.push({ x: r.u8(), y: r.u8() });
  } else {
    teleport.push({ x: TELEPORT_UNSET, y: TELEPORT_UNSET });
    teleport.push({ x: TELEPORT_UNSET, y: TELEPORT_UNSET });
  }

  const background = r.u8();

  const map: Tile[][] = [];
  for (let y = 0; y < MAPY; y++) {
    const row: Tile[] = [];
    for (let x = 0; x < MAPX; x++) {
      const tile = r.u8();
      const isPill = r.u8() !== 0;
      const isPowerpill = r.u8() !== 0;
      // En v3 le flag est stocké ; sinon on le dérive des spawns (cf. loadmap).
      const isProtected =
        fileVer >= 3 ? r.u8() !== 0 : isSpawn(x, y, player, ghosts, pickup, teleport);
      row.push({ tile, isPill, isPowerpill, isProtected });
    }
    map.push(row);
  }

  const pills = r.u16le();

  return {
    mapId,
    // Comme le jeu original, on normalise vers la version courante après chargement.
    mapVer: MAP_VER,
    validated,
    lineSet,
    player,
    ghosts: ghosts as [MapCoord, MapCoord, MapCoord, MapCoord],
    pickup,
    teleport: teleport as [MapCoord, MapCoord],
    background,
    map,
    pills,
  };
}

/** Encode un Level au format `.dp2` (toujours écrit en MAP_VER courant, comme savemap). */
export function encodeLevel(level: Level): Uint8Array {
  const size = MAP_ID.length + 1 + 1 + 1 + 2 + 8 + 2 + 4 + 1 + MAPY * MAPX * 4 + 2;
  const u8 = new Uint8Array(size);
  const view = new DataView(u8.buffer);
  let p = 0;

  for (let i = 0; i < MAP_ID.length; i++) u8[p++] = MAP_ID.charCodeAt(i) & 0xff;
  u8[p++] = MAP_VER;
  u8[p++] = level.validated ? 1 : 0;
  u8[p++] = level.lineSet & 0xff;
  u8[p++] = level.player.x & 0xff;
  u8[p++] = level.player.y & 0xff;
  for (const g of level.ghosts) {
    u8[p++] = g.x & 0xff;
    u8[p++] = g.y & 0xff;
  }
  u8[p++] = level.pickup.x & 0xff;
  u8[p++] = level.pickup.y & 0xff;
  for (const t of level.teleport) {
    u8[p++] = t.x & 0xff;
    u8[p++] = t.y & 0xff;
  }
  u8[p++] = level.background & 0xff;
  for (let y = 0; y < MAPY; y++) {
    for (let x = 0; x < MAPX; x++) {
      const tile = level.map[y][x];
      u8[p++] = tile.tile & 0xff;
      u8[p++] = tile.isPill ? 1 : 0;
      u8[p++] = tile.isPowerpill ? 1 : 0;
      u8[p++] = tile.isProtected ? 1 : 0;
    }
  }
  view.setUint16(p, level.pills & 0xffff, true);
  p += 2;

  return u8;
}
