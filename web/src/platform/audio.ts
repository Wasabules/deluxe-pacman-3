// Gestionnaire audio (WebAudio). Charge les sons OGG du jeu et les joue avec
// volume, panoramique et vitesse de lecture, plus une piste musicale en boucle.

const SND = 'Sound/';

// Noms logiques → fichier(s). Une liste = essais successifs (premier qui charge).
const FILES: Record<string, string | string[]> = {
  pill: 'Pill.ogg',
  powerpill: 'Power_Pill.ogg',
  eatGhost1: 'Eat_Ghost1.ogg',
  eatGhost2: 'Eat_Ghost2.ogg',
  eatGhost3: 'Eat_Ghost3.ogg',
  eatPickup: 'Eat_Pickup.ogg',
  pickup: 'Pickup.ogg',
  tool: 'Tool.ogg',
  newLife: 'New_Life.ogg',
  death: 'Player_Death.ogg',
  endLevel1: 'End_Level_1.ogg',
  endLevel2: 'End_Level_2.ogg',
  freeze: 'Freeze.ogg',
  heaven: 'Heaven.ogg',
  boom: 'Boom.ogg',
  bullet: 'Bullet.ogg',
  diamond: 'Diamond.ogg',
  precious: 'Precious.ogg',
  anthem: 'Anthem.ogg',
  teleport: 'Teleport.ogg',
  menuSelect: 'Menu_Select.ogg',
  menuEnter: 'Menu_Enter.ogg',
  type: 'Type.ogg',
  highScore: 'High_Score.ogg',
  background1: 'Background1.ogg',
  background2: 'Background2.ogg',
  title: ['Title.mp3', 'Title.ogg'], // mp3 ajouté par l'utilisateur, sinon ogg d'origine
};

export interface PlayOptions {
  volume?: number; // 0..1, relatif au volume effets
  pan?: number; // -1..1
  rate?: number; // vitesse de lecture (pitch)
}

const clamp01 = (v: number): number => Math.max(0, Math.min(1, v));

function loadVol(key: string, def: number): number {
  try {
    const v = localStorage.getItem(key);
    if (v !== null) {
      const n = parseFloat(v);
      if (!Number.isNaN(n)) return clamp01(n);
    }
  } catch {
    /* localStorage indisponible */
  }
  return def;
}

function saveVol(key: string, v: number): void {
  try {
    localStorage.setItem(key, String(v));
  } catch {
    /* persistance impossible */
  }
}

export class AudioManager {
  private ctx: AudioContext | null = null;
  private readonly buffers = new Map<string, AudioBuffer>();
  private sfxGain: GainNode | null = null;
  private musicGain: GainNode | null = null;

  // Volumes 0..1 (0 = muet), persistés. soundOn/musicOn en sont dérivés.
  soundVolume = loadVol('dp2.soundVol', 0.7);
  musicVolume = loadVol('dp2.musicVol', 0.6);

  private musicName: string | null = null;
  private musicSource: AudioBufferSourceNode | null = null;

  get soundOn(): boolean {
    return this.soundVolume > 0;
  }
  get musicOn(): boolean {
    return this.musicVolume > 0;
  }

  /** Crée le contexte et charge tous les sons. Sans audio, échoue silencieusement. */
  async init(): Promise<void> {
    try {
      const Ctor = window.AudioContext ?? (window as unknown as { webkitAudioContext?: typeof AudioContext }).webkitAudioContext;
      if (!Ctor) return;
      this.ctx = new Ctor();
      this.sfxGain = this.ctx.createGain();
      this.musicGain = this.ctx.createGain();
      this.sfxGain.connect(this.ctx.destination);
      this.musicGain.connect(this.ctx.destination);
      this.applyVolumes();

      await Promise.all(
        Object.entries(FILES).map(async ([name, file]) => {
          const candidates = Array.isArray(file) ? file : [file];
          for (const f of candidates) {
            try {
              const res = await fetch(SND + f);
              if (!res.ok) continue; // fichier absent → essai suivant
              const buf = await this.ctx!.decodeAudioData(await res.arrayBuffer());
              this.buffers.set(name, buf);
              break; // chargé : on arrête les essais
            } catch {
              /* échec : essai suivant, ou son ignoré */
            }
          }
        }),
      );

      // Déverrouillage à la première interaction (politique des navigateurs).
      const unlock = () => void this.ctx?.resume();
      window.addEventListener('keydown', unlock, { once: false });
      window.addEventListener('pointerdown', unlock, { once: false });
    } catch {
      this.ctx = null;
    }
  }

  private applyVolumes(): void {
    if (this.sfxGain) this.sfxGain.gain.value = this.soundVolume;
    if (this.musicGain) this.musicGain.gain.value = this.musicVolume;
  }

  /** Règle le volume des effets (0..1), persisté. 0 = muet. */
  setSoundVolume(v: number): void {
    this.soundVolume = clamp01(v);
    saveVol('dp2.soundVol', this.soundVolume);
    this.applyVolumes();
  }

  /** Règle le volume de la musique (0..1), persisté. 0 = muet (la piste continue
   *  silencieusement : remonter le volume la réentend sans coupure). */
  setMusicVolume(v: number): void {
    this.musicVolume = clamp01(v);
    saveVol('dp2.musicVol', this.musicVolume);
    this.applyVolumes();
  }

  /** Joue un effet ponctuel. */
  play(name: string, opts: PlayOptions = {}): void {
    if (!this.ctx || !this.sfxGain || this.soundVolume <= 0) return;
    const buf = this.buffers.get(name);
    if (!buf) return;
    const src = this.ctx.createBufferSource();
    src.buffer = buf;
    src.playbackRate.value = opts.rate ?? 1;
    let node: AudioNode = src;
    if (opts.pan !== undefined && this.ctx.createStereoPanner) {
      const panner = this.ctx.createStereoPanner();
      panner.pan.value = Math.max(-1, Math.min(1, opts.pan));
      src.connect(panner);
      node = panner;
    }
    const g = this.ctx.createGain();
    g.gain.value = opts.volume ?? 1;
    node.connect(g);
    g.connect(this.sfxGain);
    src.start();
  }

  /** Démarre une musique en boucle (ne fait rien si déjà en cours). */
  playMusic(name: string, rate = 1): void {
    if (!this.ctx || !this.musicGain || this.musicVolume <= 0) return;
    if (this.musicName === name) {
      if (this.musicSource) this.musicSource.playbackRate.value = rate;
      return;
    }
    this.stopMusic();
    const buf = this.buffers.get(name);
    if (!buf) return;
    const src = this.ctx.createBufferSource();
    src.buffer = buf;
    src.loop = true;
    src.playbackRate.value = rate;
    src.connect(this.musicGain);
    src.start();
    this.musicSource = src;
    this.musicName = name;
  }

  stopMusic(): void {
    if (this.musicSource) {
      try {
        this.musicSource.stop();
      } catch {
        /* déjà arrêtée */
      }
      this.musicSource = null;
    }
    this.musicName = null;
  }

  get currentMusic(): string | null {
    return this.musicName;
  }

  /** Nombre de sons décodés avec succès (diagnostic). */
  get loadedCount(): number {
    return this.buffers.size;
  }

  /** État du contexte audio (diagnostic). */
  get state(): string {
    return this.ctx?.state ?? 'none';
  }
}
