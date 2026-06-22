// Relie les événements de jeu aux sons et pilote la musique de fond.
// Lit les drapeaux transitoires du PlayState après chaque frame (sans rien
// décider de la logique : c'est de la pure présentation, comme le rendu).

import type { AudioManager } from '../platform/audio';
import type { Session, Screen } from './session';
import type { PlayState } from '../core/game';
import { Tool } from '../core/tools';
import { PLAYFIELD_W } from '../core/constants';

// Sons spécifiques à certains outils ; les autres jouent le son de ramassage.
const TOOL_SOUND: Partial<Record<Tool, string>> = {
  [Tool.SHIELDS]: 'heaven',
  [Tool.FREEZE]: 'freeze',
  [Tool.DYNAMITE]: 'boom',
  [Tool.PRECIOUS]: 'precious',
  [Tool.GOLDLEAF]: 'anthem',
};

export class AudioDirector {
  private prevScreen: Screen | null = null;
  private rng: () => number;

  constructor(private readonly audio: AudioManager, rng: () => number = Math.random) {
    this.rng = rng;
  }

  update(session: Session): void {
    const screen = session.screen;
    const play = session.play;

    if (play && (screen === 'playing' || screen === 'levelIntro')) this.playEvents(play);

    // Sons de transition d'écran.
    if (screen !== this.prevScreen) {
      if (screen === 'levelFlash') this.audio.play('endLevel1');
      else if (screen === 'gameOver') this.audio.play('death');
      this.prevScreen = screen;
    }

    this.updateMusic(session);
  }

  private pan(x: number): number {
    return (x / PLAYFIELD_W) * 2 - 1; // -1 gauche … +1 droite
  }

  private playEvents(p: PlayState): void {
    const pan = this.pan(p.pacman.x);
    const diamonds = p.toolInUse[Tool.BLUE_DIAMOND] || p.toolInUse[Tool.PINK_DIAMOND];

    if (p.atePill) this.audio.play(diamonds ? 'diamond' : 'pill', { pan });
    if (p.atePowerpill) this.audio.play('powerpill', { pan });
    if (p.ateGhost) this.audio.play(`eatGhost${1 + Math.floor(this.rng() * 3)}`, { pan });
    if (p.atePickup) this.audio.play('eatPickup', { pan });
    if (p.ateTool) this.audio.play(TOOL_SOUND[p.ateTool] ?? 'eatPickup', { pan });
    if (p.gotExtraLife) this.audio.play('newLife');
    if (p.died) this.audio.play('death', { pan });
    if (p.teleported || p.warped) this.audio.play('teleport', { pan });
    if (p.bombExploded) this.audio.play('boom', { pan });
    if (p.fired) this.audio.play('bullet', { pan });
    if (p.spawnAppeared === 'pickup') this.audio.play('pickup', { pan });
    else if (p.spawnAppeared === 'tool') this.audio.play('tool', { pan });
  }

  private updateMusic(session: Session): void {
    const s = session.screen;
    if (s === 'playing' || s === 'levelIntro' || s === 'levelFlash' || s === 'paused' || s === 'levelClear') {
      const play = session.play;
      const scared = play?.ghosts.some((g) => g.scared && !g.dead) ?? false;
      this.audio.playMusic(scared ? 'background2' : 'background1');
    } else if (s === 'intro' || s === 'menu') {
      this.audio.playMusic('title');
    } else if (s === 'hiscoreEntry' || s === 'hiscoreTable') {
      this.audio.playMusic('highScore');
    } else {
      this.audio.stopMusic();
    }
  }
}
