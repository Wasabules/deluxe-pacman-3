// Données du « codex » des outils : nom + description de chaque outil, dans les
// deux langues. Les effets sont tirés de core/game.ts (applyTool). Les 5 lettres
// EXTRA sont regroupées, et une entrée générique décrit les aliments (pickups).

import { Tool } from '../core/tools';
import { getLang, type Lang } from '../i18n';

export interface CodexEntry {
  toolImg: number; // index dans la planche d'outils (Tool - 1) ; -1 = pickup
  pickupImg?: number; // index de pickup si toolImg === -1
  name: Record<Lang, string>;
  desc: Record<Lang, string>;
}

const img = (t: Tool): number => t - 1;

export const CODEX: CodexEntry[] = [
  { toolImg: img(Tool.RANDOM), name: { en: 'Random', fr: 'Aléatoire' }, desc: { en: 'Turns into a random tool.', fr: 'Se transforme en un outil au hasard.' } },
  { toolImg: img(Tool.SHIELDS), name: { en: 'Shield', fr: 'Bouclier' }, desc: { en: 'Protects you from ghosts for a while.', fr: 'Vous protège des fantômes pendant un temps.' } },
  { toolImg: img(Tool.PRESENT), name: { en: 'Present', fr: 'Cadeau' }, desc: { en: 'Pills are worth 1000–5000 points.', fr: 'Les pastilles rapportent 1000 à 5000 points.' } },
  { toolImg: img(Tool.BLUE_DIAMOND), name: { en: 'Blue Diamond', fr: 'Diamant bleu' }, desc: { en: 'Pills are worth 1000 points each.', fr: 'Les pastilles valent 1000 points.' } },
  { toolImg: img(Tool.PINK_DIAMOND), name: { en: 'Pink Diamond', fr: 'Diamant rose' }, desc: { en: 'Pills are worth 2000 points each.', fr: 'Les pastilles valent 2000 points.' } },
  { toolImg: img(Tool.BONUS), name: { en: 'Bonus', fr: 'Bonus' }, desc: { en: 'Banks remaining pills, sends you to a bonus level.', fr: 'Empoche les pastilles restantes et part en niveau bonus.' } },
  { toolImg: img(Tool.TIMES2), name: { en: 'Times 2', fr: 'Fois 2' }, desc: { en: 'Doubles every point earned.', fr: 'Double tous les points gagnés.' } },
  { toolImg: img(Tool.TIMES5), name: { en: 'Times 5', fr: 'Fois 5' }, desc: { en: 'Multiplies every point by 5.', fr: 'Multiplie les points par 5.' } },
  { toolImg: img(Tool.TIMES7), name: { en: 'Times 7', fr: 'Fois 7' }, desc: { en: 'Multiplies every point by 7.', fr: 'Multiplie les points par 7.' } },
  { toolImg: img(Tool.FREEZE), name: { en: 'Freeze', fr: 'Gel' }, desc: { en: 'Freezes the ghosts; they cannot hurt you.', fr: 'Gèle les fantômes : ils ne peuvent plus vous toucher.' } },
  { toolImg: img(Tool.AUTISM), name: { en: 'Jackpot', fr: 'Jackpot' }, desc: { en: 'Instantly grants +30,000 points.', fr: 'Donne immédiatement +30 000 points.' } },
  { toolImg: img(Tool.PRECIOUS), name: { en: 'Precious', fr: 'Trésor' }, desc: { en: 'Instantly grants +50,000 points.', fr: 'Donne immédiatement +50 000 points.' } },
  { toolImg: img(Tool.GOLDLEAF), name: { en: 'Gold Leaf', fr: "Feuille d'or" }, desc: { en: 'Instantly grants +40,000 points.', fr: 'Donne immédiatement +40 000 points.' } },
  { toolImg: img(Tool.DYNAMITE), name: { en: 'Dynamite', fr: 'Dynamite' }, desc: { en: 'Destroys every ghost (+5000 each).', fr: 'Détruit tous les fantômes (+5000 chacun).' } },
  { toolImg: img(Tool.JUMP), name: { en: 'Jump', fr: 'Saut' }, desc: { en: 'Banks remaining pills, jumps to another level.', fr: 'Empoche les pastilles restantes et saute à un autre niveau.' } },
  { toolImg: img(Tool.EXTRA_E), name: { en: 'EXTRA letters', fr: 'Lettres EXTRA' }, desc: { en: 'Collect E-X-T-R-A to earn a free life.', fr: 'Réunissez E-X-T-R-A pour une vie bonus.' } },
  { toolImg: img(Tool.GUN), name: { en: 'Gun', fr: 'Pistolet' }, desc: { en: 'Press SPACE to shoot the ghosts.', fr: 'Appuyez sur ESPACE pour abattre les fantômes.' } },
  { toolImg: img(Tool.GLUE), name: { en: 'Glue', fr: 'Colle' }, desc: { en: 'Slows the ghosts to half speed.', fr: 'Ralentit les fantômes de moitié.' } },
  { toolImg: img(Tool.SPEED), name: { en: 'Speed', fr: 'Vitesse' }, desc: { en: 'Makes Pacman move faster.', fr: 'Accélère les déplacements de Pacman.' } },
  { toolImg: img(Tool.TIME), name: { en: 'Time', fr: 'Temps' }, desc: { en: 'Extends how long tool effects last.', fr: 'Prolonge la durée des effets des outils.' } },
  { toolImg: img(Tool.SKULL), name: { en: 'Skull', fr: 'Crâne' }, desc: { en: 'DANGER! Kills you instantly.', fr: 'DANGER ! Vous tue sur le coup.' } },
  // --- Outils ajoutés (icônes dessinées) ---
  { toolImg: img(Tool.MAGNET), name: { en: 'Magnet', fr: 'Aimant' }, desc: { en: 'Attracts nearby pills toward you.', fr: 'Attire les pastilles proches vers vous.' } },
  { toolImg: img(Tool.PANIC), name: { en: 'Panic', fr: 'Panique' }, desc: { en: 'The ghosts flee from you for a while.', fr: 'Les fantômes vous fuient un moment.' } },
  { toolImg: img(Tool.PHASE), name: { en: 'Phase', fr: 'Phase' }, desc: { en: 'Walk through walls for a while.', fr: 'Traversez les murs pendant un temps.' } },
  { toolImg: img(Tool.BOMB), name: { en: 'Bomb', fr: 'Bombe' }, desc: { en: 'Drops a bomb that blasts nearby ghosts. Keep clear!', fr: 'Pose une bombe qui détruit les fantômes autour. Écartez-vous !' } },
  { toolImg: img(Tool.LIGHTNING), name: { en: 'Lightning', fr: 'Éclair' }, desc: { en: 'Strikes down the nearest ghost.', fr: 'Foudroie le fantôme le plus proche.' } },
  { toolImg: img(Tool.WARP), name: { en: 'Warp', fr: 'Téléport' }, desc: { en: 'Teleports you to a random open spot.', fr: 'Vous téléporte sur une case libre au hasard.' } },
  { toolImg: -1, pickupImg: 6, name: { en: 'Food', fr: 'Aliments' }, desc: { en: 'Snacks give points and refill energy.', fr: "Les aliments donnent des points et rechargent l'énergie." } },
];

export const CODEX_PER_PAGE = 12; // 2 colonnes × 6 lignes
export const codexPageCount = (): number => Math.ceil(CODEX.length / CODEX_PER_PAGE);

export const codexName = (e: CodexEntry): string => e.name[getLang()];
export const codexDesc = (e: CodexEntry): string => e.desc[getLang()];
