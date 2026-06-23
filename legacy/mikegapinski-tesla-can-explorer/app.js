"use strict";

const DATA_SOURCES = {
  mcu2: {
    label: "Model 3 MCU2 (Intel)",
    url: "./data/can_frames_decoded_all_values_mcu2.json",
  },
  mcu3: {
    label: "Model 3 MCU3 (AMD)",
    url: "./data/can_frames_decoded_all_values_mcu3.json",
  },
  modelsx_amd: {
    label: "Model S/X MCU3 (AMD)",
    url: "./data/can_frames_decoded_all_values_modelsx_amd.json",
  },
  modelsx_intel: {
    label: "Model S/X MCU2 (Intel)",
    url: "./data/can_frames_decoded_all_values_modelsx_intel.json",
  },
};
const DEFAULT_SOURCE_KEY = "mcu2";
const NUMBER_FMT = new Intl.NumberFormat("en-US");

const els = {
  statFrames: document.getElementById("stat-frames"),
  statSignals: document.getElementById("stat-signals"),
  statValues: document.getElementById("stat-values"),
  statVapi: document.getElementById("stat-vapi"),
  datasetSourceLabel: document.getElementById("dataset-source-label"),
  datasetLibsLabel: document.getElementById("dataset-libs-label"),
  frameResults: document.getElementById("frame-results"),
  frameList: document.getElementById("frame-list"),
  frameTitle: document.getElementById("frame-title"),
  frameSubtitle: document.getElementById("frame-subtitle"),
  frameMeta: document.getElementById("frame-meta"),
  dataSource: document.getElementById("data-source"),
  globalSearch: document.getElementById("global-search"),
  signalSearch: document.getElementById("signal-search"),
  busFilter: document.getElementById("bus-filter"),
  moduleFilter: document.getElementById("module-filter"),
  sortMode: document.getElementById("sort-mode"),
  enumeratedOnly: document.getElementById("enumerated-only"),
  signalPageSize: document.getElementById("signal-page-size"),
  signalPagination: document.getElementById("signal-pagination"),
  signalTableWrap: document.getElementById("signal-table-wrap"),
  loadingOverlay: document.getElementById("loading-overlay"),
  loadingText: document.getElementById("loading-text"),
  valuesTemplate: document.getElementById("signal-values-template"),
};

const state = {
  payload: null,
  frames: [],
  frameByKey: new Map(),
  filteredFrames: [],
  selectedKey: "",
  signalQuery: "",
  signalPage: 1,
  signalPageSize: 200,
  expandedSignalKeys: new Set(),
  frameFilterTimer: null,
  dataSelection: null,
};

function escapeHtml(value) {
  return String(value ?? "")
    .replaceAll("&", "&amp;")
    .replaceAll("<", "&lt;")
    .replaceAll(">", "&gt;")
    .replaceAll('"', "&quot;")
    .replaceAll("'", "&#39;");
}

function formatNum(v) {
  return NUMBER_FMT.format(Number(v) || 0);
}

function getSelectionFromQuery() {
  const params = new URLSearchParams(window.location.search);
  const dataOverride = params.get("data");
  const querySource = params.get("source") || DEFAULT_SOURCE_KEY;
  const sourceKey = DATA_SOURCES[querySource] ? querySource : DEFAULT_SOURCE_KEY;
  const source = DATA_SOURCES[sourceKey];
  if (dataOverride) {
    return {
      sourceKey,
      sourceLabel: source.label,
      dataUrl: dataOverride,
      isDataOverride: true,
    };
  }
  return {
    sourceKey,
    sourceLabel: source.label,
    dataUrl: source.url,
    isDataOverride: false,
  };
}

function applySourceToQuery(sourceKey) {
  const params = new URLSearchParams(window.location.search);
  params.set("source", sourceKey);
  params.delete("data");
  const next = `${window.location.pathname}?${params.toString()}`;
  window.location.assign(next);
}

function initSourceSelector(selection) {
  if (!els.dataSource) {
    return;
  }

  const entries = Object.entries(DATA_SOURCES);
  els.dataSource.innerHTML = entries
    .map(
      ([key, source]) =>
        `<option value="${escapeHtml(key)}">${escapeHtml(source.label)}</option>`
    )
    .join("");
  els.dataSource.value = selection.sourceKey;

  if (selection.isDataOverride) {
    els.dataSource.title = "Disabled because a custom ?data= override is active.";
    els.dataSource.disabled = true;
  }
}

function tokenize(text) {
  return String(text || "")
    .toLowerCase()
    .trim()
    .split(/\s+/)
    .filter(Boolean);
}

function frameKey(frame) {
  return `${frame.bus_name}|${frame.bus_id}|${frame.address_dec}|${frame.frame_name}`;
}

function getModuleName(frameName) {
  if (!frameName) {
    return "UNKNOWN";
  }
  const idx = frameName.indexOf("_");
  if (idx === -1) {
    return frameName;
  }
  return frameName.slice(0, idx);
}

async function loadPayload() {
  const selection = state.dataSelection || getSelectionFromQuery();
  const url = selection.dataUrl;
  setLoading(`Loading ${url}`);
  const response = await fetch(url);
  if (!response.ok) {
    throw new Error(`Failed to load ${url}: HTTP ${response.status}`);
  }
  return response.json();
}

async function indexFrames(frames) {
  const busSet = new Set();
  const moduleSet = new Set();
  let totalSignals = 0;
  let totalValues = 0;
  let totalVapiAliases = 0;

  for (let i = 0; i < frames.length; i += 1) {
    const frame = frames[i];
    const key = frameKey(frame);
    frame.__key = key;
    frame.__module = getModuleName(frame.frame_name);
    frame.__signals = Array.isArray(frame.signals) ? frame.signals : [];
    frame.__signalCount = frame.__signals.length;
    frame.__enumeratedSignalCount = 0;
    frame.__valueCount = 0;
    frame.__vapiAliasCount = 0;

    const searchBase = [
      frame.bus_name,
      frame.bus_id,
      frame.address_hex,
      frame.address_dec,
      frame.frame_name,
      frame.__module,
    ];
    const signalSearchParts = [];
    const labelSearchParts = [];

    for (const signal of frame.__signals) {
      const signalName = String(signal.signal_name || "");
      const enumMap = String(signal.enum_map_symbol || "");
      const note = String(signal.possible_values_note || "");
      const vapiAlias = String(signal.vapi_alias || "");
      const vapiSource = String(signal.vapi_source || "");
      const values = Array.isArray(signal.possible_values)
        ? signal.possible_values
        : [];

      signalSearchParts.push(signalName, enumMap, note, vapiAlias, vapiSource);

      if (vapiAlias) {
        frame.__vapiAliasCount += 1;
      }

      if (values.length > 0) {
        frame.__enumeratedSignalCount += 1;
        frame.__valueCount += values.length;
        for (const value of values) {
          if (value && value.label != null) {
            labelSearchParts.push(String(value.label));
          }
        }
      }
    }

    frame.__search =
      `${searchBase.join(" ")} ${signalSearchParts.join(" ")}`.toLowerCase();
    frame.__labelSearch = labelSearchParts.join(" ").toLowerCase();

    busSet.add(`${frame.bus_name} (${frame.bus_id})`);
    moduleSet.add(frame.__module);
    totalSignals += frame.__signalCount;
    totalValues += frame.__valueCount;
    totalVapiAliases += frame.__vapiAliasCount;
    state.frameByKey.set(key, frame);

    if (i % 20 === 0) {
      setLoading(`Indexing frames ${i + 1}/${frames.length}`);
      await new Promise((resolve) => setTimeout(resolve, 0));
    }
  }

  return {
    totalSignals,
    totalValues,
    totalVapiAliases,
    buses: [...busSet].sort((a, b) => a.localeCompare(b)),
    modules: [...moduleSet].sort((a, b) => a.localeCompare(b)),
  };
}

function applyFilterOptions(buses, modules) {
  for (const bus of buses) {
    const opt = document.createElement("option");
    opt.value = bus;
    opt.textContent = bus;
    els.busFilter.appendChild(opt);
  }
  for (const moduleName of modules) {
    const opt = document.createElement("option");
    opt.value = moduleName;
    opt.textContent = moduleName;
    els.moduleFilter.appendChild(opt);
  }
}

function frameMatches(frame, tokens, busFilter, moduleFilter, onlyEnumerated) {
  if (busFilter && `${frame.bus_name} (${frame.bus_id})` !== busFilter) {
    return false;
  }
  if (moduleFilter && frame.__module !== moduleFilter) {
    return false;
  }
  if (onlyEnumerated && frame.__enumeratedSignalCount === 0) {
    return false;
  }

  if (tokens.length === 0) {
    return true;
  }

  for (const token of tokens) {
    if (frame.__search.includes(token)) {
      continue;
    }
    if (frame.__labelSearch && frame.__labelSearch.includes(token)) {
      continue;
    }
    return false;
  }
  return true;
}

function sortFrames(list, mode) {
  list.sort((a, b) => {
    if (mode === "name") {
      return String(a.frame_name).localeCompare(String(b.frame_name));
    }
    if (mode === "signals") {
      if (b.__signalCount !== a.__signalCount) {
        return b.__signalCount - a.__signalCount;
      }
      return a.address_dec - b.address_dec;
    }
    if (mode === "enums") {
      if (b.__enumeratedSignalCount !== a.__enumeratedSignalCount) {
        return b.__enumeratedSignalCount - a.__enumeratedSignalCount;
      }
      return a.address_dec - b.address_dec;
    }
    if (mode === "vapi") {
      if (b.__vapiAliasCount !== a.__vapiAliasCount) {
        return b.__vapiAliasCount - a.__vapiAliasCount;
      }
      return a.address_dec - b.address_dec;
    }
    if (a.address_dec !== b.address_dec) {
      return a.address_dec - b.address_dec;
    }
    return String(a.frame_name).localeCompare(String(b.frame_name));
  });
}

function applyFrameFilters() {
  const tokens = tokenize(els.globalSearch.value);
  const busFilter = els.busFilter.value;
  const moduleFilter = els.moduleFilter.value;
  const sortMode = els.sortMode.value;
  const onlyEnumerated = els.enumeratedOnly.checked;

  const nextFrames = state.frames.filter((frame) =>
    frameMatches(frame, tokens, busFilter, moduleFilter, onlyEnumerated)
  );

  sortFrames(nextFrames, sortMode);
  state.filteredFrames = nextFrames;
  ensureSelectedFrame();
  renderFrameList();
  renderFrameDetail();
}

function ensureSelectedFrame() {
  if (!state.filteredFrames.length) {
    state.selectedKey = "";
    return;
  }
  if (
    state.selectedKey &&
    state.filteredFrames.some((frame) => frame.__key === state.selectedKey)
  ) {
    return;
  }
  state.selectedKey = state.filteredFrames[0].__key;
  state.signalPage = 1;
  state.expandedSignalKeys.clear();
}

function renderFrameList() {
  if (!state.filteredFrames.length) {
    els.frameList.innerHTML =
      '<div class="empty-state">No frames match your filters.</div>';
    els.frameResults.textContent = "0 frames";
    return;
  }

  const html = state.filteredFrames
    .map((frame) => {
      const isActive = frame.__key === state.selectedKey;
      return `
        <article class="frame-item ${isActive ? "active" : ""}" data-frame-key="${escapeHtml(frame.__key)}">
          <div class="frame-top">
            <div class="frame-name">${escapeHtml(frame.frame_name)}</div>
            <div class="frame-addr">${escapeHtml(frame.address_hex)}</div>
          </div>
          <div class="frame-meta">
            <span class="meta-pill">Bus ${escapeHtml(frame.bus_name)} (${escapeHtml(frame.bus_id)})</span>
            <span class="meta-pill">${escapeHtml(frame.__module)}</span>
            <span class="meta-pill">${formatNum(frame.__signalCount)} signals</span>
            <span class="meta-pill">${formatNum(frame.__enumeratedSignalCount)} enum</span>
            <span class="meta-pill">${formatNum(frame.__vapiAliasCount)} VAPI</span>
          </div>
        </article>
      `;
    })
    .join("");

  els.frameList.innerHTML = html;
  els.frameResults.textContent = `${formatNum(state.filteredFrames.length)} frames`;
}

function getSelectedFrame() {
  return state.frameByKey.get(state.selectedKey) || null;
}

function getSignalSearchBlob(signal) {
  if (signal.__searchBlob) {
    return signal.__searchBlob;
  }
  const valueLabels = Array.isArray(signal.possible_values)
    ? signal.possible_values
        .map((v) => `${v.value_dec ?? ""} ${v.value_hex ?? ""} ${v.label ?? ""}`)
        .join(" ")
    : "";
  signal.__searchBlob = `${signal.signal_name || ""} ${signal.enum_map_symbol || ""} ${
    signal.possible_values_note || ""
  } ${signal.vapi_alias || ""} ${signal.vapi_source || ""} ${valueLabels}`.toLowerCase();
  return signal.__searchBlob;
}

function filterSignals(signals, query) {
  const tokens = tokenize(query);
  if (!tokens.length) {
    return signals;
  }
  return signals.filter((signal) => {
    const blob = getSignalSearchBlob(signal);
    return tokens.every((token) => blob.includes(token));
  });
}

function renderFrameDetail() {
  const frame = getSelectedFrame();
  if (!frame) {
    els.frameTitle.textContent = "Select a frame";
    els.frameSubtitle.textContent =
      "Browse by frame address and signal values.";
    els.frameMeta.innerHTML = "";
    els.signalPagination.innerHTML = "";
    els.signalTableWrap.innerHTML =
      '<div class="empty-state">Frame details will appear here after selection.</div>';
    els.signalSearch.disabled = true;
    els.signalPageSize.disabled = true;
    return;
  }

  els.signalSearch.disabled = false;
  els.signalPageSize.disabled = false;
  els.frameTitle.textContent = `${frame.frame_name} (${frame.address_hex})`;
  els.frameSubtitle.textContent = `Address ${frame.address_dec} on ${frame.bus_name} bus`;
  els.frameMeta.innerHTML = [
    `<span class="chip">Bus: <code>${escapeHtml(frame.bus_name)}</code> (${escapeHtml(frame.bus_id)})</span>`,
    `<span class="chip">Module: <code>${escapeHtml(frame.__module)}</code></span>`,
    `<span class="chip">Signals: <code>${formatNum(frame.__signalCount)}</code></span>`,
    `<span class="chip">Enumerated: <code>${formatNum(frame.__enumeratedSignalCount)}</code></span>`,
    `<span class="chip">Enum Values: <code>${formatNum(frame.__valueCount)}</code></span>`,
    `<span class="chip">VAPI Aliases: <code>${formatNum(frame.__vapiAliasCount)}</code></span>`,
  ].join("");

  const filteredSignals = filterSignals(frame.__signals, state.signalQuery);
  const total = filteredSignals.length;
  const pageSize = state.signalPageSize;
  const totalPages = Math.max(1, Math.ceil(total / pageSize));
  if (state.signalPage > totalPages) {
    state.signalPage = totalPages;
  }
  const start = (state.signalPage - 1) * pageSize;
  const end = Math.min(total, start + pageSize);
  const pageSignals = filteredSignals.slice(start, end);

  renderSignalPagination(total, totalPages, start, end);
  renderSignalTable(frame, pageSignals);
}

function renderSignalPagination(total, totalPages, start, end) {
  const startNum = total === 0 ? 0 : start + 1;
  const endNum = total === 0 ? 0 : end;
  els.signalPagination.innerHTML = `
    <div>Showing ${formatNum(startNum)}-${formatNum(endNum)} of ${formatNum(total)} signals</div>
    <div class="pager-buttons">
      <button type="button" data-page="first" ${state.signalPage === 1 ? "disabled" : ""}>First</button>
      <button type="button" data-page="prev" ${state.signalPage === 1 ? "disabled" : ""}>Prev</button>
      <span>Page ${formatNum(state.signalPage)} / ${formatNum(totalPages)}</span>
      <button type="button" data-page="next" ${state.signalPage >= totalPages ? "disabled" : ""}>Next</button>
      <button type="button" data-page="last" ${state.signalPage >= totalPages ? "disabled" : ""}>Last</button>
    </div>
  `;
}

function renderSignalTable(frame, signals) {
  if (!signals.length) {
    els.signalTableWrap.innerHTML =
      '<div class="empty-state">No signals match the current filter.</div>';
    return;
  }

  const rows = [];
  for (const signal of signals) {
    const signalKey = `${frame.__key}:${signal.signal_index}`;
    const values = Array.isArray(signal.possible_values) ? signal.possible_values : [];
    const isExpanded = state.expandedSignalKeys.has(signalKey);
    const vapiAlias = String(signal.vapi_alias || "");
    const vapiSource = String(signal.vapi_source || "");
    const noteClass =
      values.length === 0 && signal.enum_map_symbol ? "status-note warn" : "status-note";

    rows.push(`
      <tr class="signal-row" data-signal-key="${escapeHtml(signalKey)}">
        <td class="mono">${escapeHtml(signal.signal_index)}</td>
        <td class="signal-name">${escapeHtml(signal.signal_name)}</td>
        <td class="mono">${escapeHtml(signal.enum_map_symbol || "-")}</td>
        <td class="mono">
          ${escapeHtml(vapiAlias || "-")}
          ${vapiSource ? `<div class="vapi-source">${escapeHtml(vapiSource)}</div>` : ""}
        </td>
        <td class="mono">${formatNum(values.length)}</td>
        <td class="${noteClass}">${escapeHtml(signal.possible_values_note || "")}</td>
      </tr>
    `);

    if (isExpanded) {
      rows.push(`
        <tr>
          <td colspan="6">${renderValuesTable(values)}</td>
        </tr>
      `);
    }
  }

  els.signalTableWrap.innerHTML = `
    <table class="signal-table">
      <thead>
        <tr>
          <th>#</th>
          <th>Signal Name</th>
          <th>Enum Map</th>
          <th>VAPI Alias</th>
          <th>Value Count</th>
          <th>Notes</th>
        </tr>
      </thead>
      <tbody>${rows.join("")}</tbody>
    </table>
  `;
}

function renderValuesTable(values) {
  if (!values.length) {
    return '<div class="empty-state">No discrete values decoded for this signal.</div>';
  }
  const rowHtml = values
    .map(
      (value) => `
        <tr>
          <td class="mono">${escapeHtml(value.value_dec)}</td>
          <td class="mono">${escapeHtml(value.value_hex)}</td>
          <td>${escapeHtml(value.label)}</td>
        </tr>
      `
    )
    .join("");

  return `
    <div class="values-wrap">
      <table class="values-table">
        <thead>
          <tr>
            <th>Value (Dec)</th>
            <th>Value (Hex)</th>
            <th>Label</th>
          </tr>
        </thead>
        <tbody>${rowHtml}</tbody>
      </table>
    </div>
  `;
}

function selectFrame(key) {
  if (key === state.selectedKey) {
    return;
  }
  state.selectedKey = key;
  state.signalPage = 1;
  state.signalQuery = "";
  state.expandedSignalKeys.clear();
  els.signalSearch.value = "";
  renderFrameList();
  renderFrameDetail();
}

function setLoading(message) {
  els.loadingText.textContent = message;
}

function hideLoading() {
  els.loadingOverlay.classList.add("hidden");
}

function showFatalError(message) {
  setLoading(message);
  els.loadingOverlay.classList.remove("hidden");
}

function bindEvents() {
  if (els.dataSource) {
    els.dataSource.addEventListener("change", () => {
      const key = els.dataSource.value;
      if (DATA_SOURCES[key]) {
        applySourceToQuery(key);
      }
    });
  }

  els.globalSearch.addEventListener("input", () => {
    if (state.frameFilterTimer) {
      clearTimeout(state.frameFilterTimer);
    }
    state.frameFilterTimer = setTimeout(() => {
      applyFrameFilters();
    }, 130);
  });

  els.busFilter.addEventListener("change", applyFrameFilters);
  els.moduleFilter.addEventListener("change", applyFrameFilters);
  els.sortMode.addEventListener("change", applyFrameFilters);
  els.enumeratedOnly.addEventListener("change", applyFrameFilters);

  els.frameList.addEventListener("click", (event) => {
    const row = event.target.closest("[data-frame-key]");
    if (!row) {
      return;
    }
    selectFrame(row.getAttribute("data-frame-key") || "");
  });

  els.signalSearch.addEventListener("input", () => {
    state.signalQuery = els.signalSearch.value || "";
    state.signalPage = 1;
    state.expandedSignalKeys.clear();
    renderFrameDetail();
  });

  els.signalPageSize.addEventListener("change", () => {
    state.signalPageSize = Number(els.signalPageSize.value) || 200;
    state.signalPage = 1;
    renderFrameDetail();
  });

  els.signalPagination.addEventListener("click", (event) => {
    const btn = event.target.closest("button[data-page]");
    if (!btn) {
      return;
    }
    const frame = getSelectedFrame();
    if (!frame) {
      return;
    }
    const filteredSignals = filterSignals(frame.__signals, state.signalQuery);
    const totalPages = Math.max(
      1,
      Math.ceil(filteredSignals.length / state.signalPageSize)
    );

    const mode = btn.getAttribute("data-page");
    if (mode === "first") state.signalPage = 1;
    if (mode === "prev") state.signalPage = Math.max(1, state.signalPage - 1);
    if (mode === "next")
      state.signalPage = Math.min(totalPages, state.signalPage + 1);
    if (mode === "last") state.signalPage = totalPages;
    renderFrameDetail();
  });

  els.signalTableWrap.addEventListener("click", (event) => {
    const row = event.target.closest(".signal-row[data-signal-key]");
    if (!row) {
      return;
    }
    const key = row.getAttribute("data-signal-key") || "";
    if (!key) {
      return;
    }
    if (state.expandedSignalKeys.has(key)) {
      state.expandedSignalKeys.delete(key);
    } else {
      state.expandedSignalKeys.add(key);
    }
    renderFrameDetail();
  });
}

async function init() {
  state.dataSelection = getSelectionFromQuery();
  initSourceSelector(state.dataSelection);
  bindEvents();
  try {
    const payload = await loadPayload();
    state.payload = payload;
    state.frames = Array.isArray(payload.frames) ? payload.frames : [];

    if (payload.dataset_source && els.datasetSourceLabel) {
      const vehicle = escapeHtml(payload.dataset_source.vehicle || "Model 3");
      const firmware = escapeHtml(payload.dataset_source.firmware || "2026.2");
      const mcu = escapeHtml(payload.dataset_source.mcu || "");
      const soc = escapeHtml(payload.dataset_source.soc || "");
      const hw = `${mcu} ${soc}`.trim();
      const hwText = hw ? ` (${hw})` : "";
      els.datasetSourceLabel.innerHTML = `Dataset source: ${vehicle} firmware <code>${firmware}</code>${hwText}`;
    }
    if (els.datasetLibsLabel) {
      const libs = ["libQtCarCANData.so"];
      const processed = Array.isArray(payload.sources_processed)
        ? payload.sources_processed
        : [];
      for (const item of processed) {
        const lib = item && typeof item.library === "string" ? item.library : "";
        if (lib) {
          libs.push(lib);
        }
      }
      const unique = [...new Set(libs)].filter(Boolean);
      if (unique.length > 0) {
        els.datasetLibsLabel.innerHTML = `Sources: ${unique
          .map((lib) => `<code>${escapeHtml(lib)}</code>`)
          .join(", ")}`;
      }
    }

    setLoading("Building search index...");
    const { totalSignals, totalValues, totalVapiAliases, buses, modules } = await indexFrames(
      state.frames
    );

    applyFilterOptions(buses, modules);
    els.statFrames.textContent = formatNum(state.frames.length);
    els.statSignals.textContent = formatNum(totalSignals);
    els.statValues.textContent = formatNum(totalValues);
    if (els.statVapi) {
      let vapiCount = totalVapiAliases;
      const digestCount =
        payload &&
        payload.vapi_digest &&
        payload.vapi_digest.counts &&
        Number(payload.vapi_digest.counts.db_signals_annotated_with_vapi_alias);
      if (Number.isFinite(digestCount) && digestCount >= 0) {
        vapiCount = digestCount;
      }
      els.statVapi.textContent = formatNum(vapiCount);
    }

    state.signalPageSize = Number(els.signalPageSize.value) || 200;
    applyFrameFilters();
    hideLoading();
  } catch (error) {
    console.error(error);
    showFatalError(
      `Failed to load data. Start a local web server in this portal directory and open /. Error: ${String(
        error.message || error
      )}`
    );
  }
}

init();
