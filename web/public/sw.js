// Service worker pour jouer hors-ligne après la première visite.
//
// Stratégie :
//  - documents (navigations) et modules JS/CSS  → network-first
//    (on récupère toujours la dernière version ; le cache sert de secours offline).
//  - assets lourds et immuables (images, sons, polices, niveaux) → cache-first.
//  - sw.js lui-même n'est jamais intercepté (le navigateur le revalide).
//
// Le numéro de version invalide les anciens caches au déploiement.
const CACHE = 'dp2-v2';

self.addEventListener('install', () => self.skipWaiting());

self.addEventListener('activate', (e) => {
  e.waitUntil(
    (async () => {
      const keys = await caches.keys();
      await Promise.all(keys.filter((k) => k !== CACHE).map((k) => caches.delete(k)));
      await self.clients.claim();
    })(),
  );
});

function isImmutableAsset(url) {
  return /\/(Graphics|Sound|Fonts|Levels)\//.test(url.pathname) || url.pathname.endsWith('.webmanifest');
}

self.addEventListener('fetch', (e) => {
  const req = e.request;
  const url = new URL(req.url);
  if (req.method !== 'GET' || url.origin !== self.location.origin) return;
  if (url.pathname.endsWith('/sw.js')) return; // ne jamais se mettre soi-même en cache

  if (isImmutableAsset(url)) {
    // cache-first : ces fichiers ne changent pas.
    e.respondWith(
      (async () => {
        const cache = await caches.open(CACHE);
        const cached = await cache.match(req);
        if (cached) return cached;
        const res = await fetch(req);
        if (res.ok) cache.put(req, res.clone());
        return res;
      })(),
    );
  } else {
    // network-first : HTML, JS, CSS — toujours la dernière version si en ligne.
    e.respondWith(
      (async () => {
        const cache = await caches.open(CACHE);
        try {
          const res = await fetch(req);
          if (res.ok) cache.put(req, res.clone());
          return res;
        } catch (err) {
          const cached = await cache.match(req);
          if (cached) return cached;
          throw err;
        }
      })(),
    );
  }
});
