export const DEFAULT_ROUTE = '/setup';

export const APP_ROUTES = [
  {
    path: '/setup',
    label: 'Setup',
    title: 'TeslaCANModder | Setup',
    accent: false,
  },
  {
    path: '/dashboard',
    label: 'Dashboard',
    title: 'TeslaCANModder | Dashboard',
    accent: false,
  },
  {
    path: '/explorer',
    label: 'CAN Explorer',
    title: 'TeslaCANModder | CAN Explorer',
    accent: false,
  },
  {
    path: '/flash',
    label: 'Flash',
    title: 'TeslaCANModder | Flash',
    accent: true,
  },
];

function normalizePathname(pathname) {
  if (!pathname || pathname === '/') {
    return '/';
  }

  return pathname.endsWith('/') ? pathname.slice(0, -1) : pathname;
}

export function findRouteMeta(pathname) {
  const normalizedPathname = normalizePathname(pathname);
  return APP_ROUTES.find((route) => route.path === normalizedPathname) || null;
}
