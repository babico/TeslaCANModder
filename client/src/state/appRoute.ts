import { useEffect, useState } from "react";

import { DEFAULT_DOC_ROUTE } from "../generated/docsContent";

export type AppTabRoute = "dashboard" | "controls" | "console" | "flasher" | "docs";

export interface AppRouteState {
  tab: AppTabRoute;
  docRoute: string;
}

const VALID_TABS: AppTabRoute[] = ["dashboard", "controls", "console", "flasher", "docs"];

function normalizeTab(raw: string | undefined): AppTabRoute {
  if (raw === "monitor") {
    return "console";
  }
  if (raw === "drive") {
    return "dashboard";
  }
  return VALID_TABS.includes(raw as AppTabRoute) ? (raw as AppTabRoute) : "dashboard";
}

function getDefaultRoute(): AppRouteState {
  return {
    tab: "dashboard",
    docRoute: DEFAULT_DOC_ROUTE,
  };
}

function parseHashRoute(hash: string): AppRouteState {
  const cleaned = hash.replace(/^#/, "").replace(/^\//, "").trim();
  if (!cleaned) {
    return getDefaultRoute();
  }

  const segments = cleaned.split("/").filter(Boolean);
  const tab = normalizeTab(segments[0]);
  if (tab !== "docs") {
    return {
      tab,
      docRoute: DEFAULT_DOC_ROUTE,
    };
  }

  return {
    tab,
    docRoute: segments.slice(1).join("/"),
  };
}

function buildHashRoute(route: AppRouteState): string {
  if (route.tab !== "docs") {
    return `#/${route.tab}`;
  }

  return route.docRoute ? `#/docs/${route.docRoute}` : "#/docs";
}

export function useAppRouteState() {
  const [route, setRoute] = useState<AppRouteState>(() => {
    if (typeof window === "undefined" || !window.location) {
      return getDefaultRoute();
    }
    return parseHashRoute(window.location.hash);
  });

  useEffect(() => {
    if (typeof window === "undefined" || !window.location) {
      return undefined;
    }

    const handleHashChange = () => {
      setRoute(parseHashRoute(window.location.hash));
    };

    window.addEventListener("hashchange", handleHashChange);
    return () => {
      window.removeEventListener("hashchange", handleHashChange);
    };
  }, []);

  const navigate = (nextRoute: AppRouteState) => {
    setRoute(nextRoute);

    if (typeof window === "undefined" || !window.location) {
      return;
    }

    const nextHash = buildHashRoute(nextRoute);
    if (window.location.hash !== nextHash) {
      window.location.hash = nextHash;
    }
  };

  return {
    route,
    navigateToTab: (tab: AppTabRoute) => {
      navigate({
        tab,
        docRoute: route.docRoute || DEFAULT_DOC_ROUTE,
      });
    },
    navigateToDoc: (docRoute: string) => {
      navigate({
        tab: "docs",
        docRoute,
      });
    },
  };
}
