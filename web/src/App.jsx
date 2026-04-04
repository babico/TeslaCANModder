import { useEffect } from 'react';
import { Navigate, NavLink, Route, Routes, useLocation } from 'react-router-dom';

import { APP_ROUTES, DEFAULT_ROUTE, findRouteMeta } from './app/routes';
import { useBoardLink } from './hooks/useBoardLink';
import DashboardPage from './pages/DashboardPage';
import ExplorerPage from './pages/ExplorerPage';
import FlashPage from './pages/FlashPage';
import SetupPage from './pages/SetupPage';

const PAGE_COMPONENTS = {
  '/setup': SetupPage,
  '/dashboard': DashboardPage,
  '/explorer': ExplorerPage,
  '/flash': FlashPage,
};

function App() {
  const board = useBoardLink();
  const location = useLocation();
  const routeMeta = findRouteMeta(location.pathname);

  useEffect(() => {
    document.title = routeMeta?.title || 'TeslaCANModder';
  }, [routeMeta]);

  return (
    <div className="app-wrapper">
      <div className="bg-noise"></div>
      <div className="bg-orb bg-orb-a"></div>
      <div className="bg-orb bg-orb-b"></div>
      <div className="bg-orb bg-orb-c"></div>

      <header className="topbar">
        <div className="brand">
          <div className="brand-icon">
            <svg viewBox="0 0 32 32" width="28" height="28" fill="none">
              <path d="M16 4C9.4 4 4 6 4 6s2.6 2 12 2 12-2 12-2-5.4-2-12-2z" fill="#e82127" />
              <path d="M16 10c-3.2 0-5.5-.3-7-.6V28l7-4 7 4V9.4c-1.5.3-3.8.6-7 .6z" fill="#e82127" />
            </svg>
          </div>
          <div className="brand-text">
            <span className="brand-kicker">Open Source Toolkit</span>
            <h1>TeslaCANModder</h1>
          </div>
        </div>

        <nav className="app-nav" aria-label="Primary">
          {APP_ROUTES.map((route) => (
            <NavLink
              key={route.path}
              to={route.path}
              className={({ isActive }) => [
                'app-nav-link',
                isActive ? 'active' : '',
                route.accent ? 'app-nav-link--accent' : '',
              ].filter(Boolean).join(' ')}
            >
              {route.label}
            </NavLink>
          ))}
        </nav>
      </header>

      <main className="main-content">
        <Routes>
          <Route path="/" element={<Navigate to={DEFAULT_ROUTE} replace />} />
          {APP_ROUTES.map((route) => {
            const PageComponent = PAGE_COMPONENTS[route.path];

            return (
              <Route
                key={route.path}
                path={route.path}
                element={<PageComponent board={board} />}
              />
            );
          })}
          <Route path="*" element={<Navigate to={DEFAULT_ROUTE} replace />} />
        </Routes>
      </main>

      <footer className="site-footer">
        <span>TeslaCANModder Web Client · Modular Viewer + Packages · Not affiliated with Tesla, Inc.</span>
      </footer>
    </div>
  );
}

export default App;
