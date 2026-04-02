import { useCallback, useEffect, useId, useRef, useState } from 'react';

let mermaidReady = false;
let mermaidPromise;

async function getMermaid() {
  if (!mermaidPromise) {
    mermaidPromise = import('mermaid').then(({ default: mermaid }) => {
      if (!mermaidReady) {
        mermaid.initialize({
          startOnLoad: false,
          securityLevel: 'strict',
          theme: 'base',
          themeVariables: {
            background: '#10131a',
            primaryColor: '#131720',
            primaryTextColor: '#eef0f6',
            primaryBorderColor: '#334155',
            lineColor: '#94a3b8',
            secondaryColor: '#172033',
            tertiaryColor: '#0f172a',
            clusterBkg: '#0f1520',
            clusterBorder: '#334155',
            fontFamily: 'Inter, Segoe UI, sans-serif',
            fontSize: '14px',
          },
          flowchart: {
            htmlLabels: true,
            curve: 'basis',
            nodeSpacing: 34,
            rankSpacing: 38,
            padding: 16,
            useMaxWidth: false,
          },
        });

        mermaidReady = true;
      }

      return mermaid;
    });
  }

  return mermaidPromise;
}

const MIN_SCALE = 0.45;
const MAX_SCALE = 2.4;
const ZOOM_STEP = 0.18;

function clamp(value, min, max) {
  return Math.min(Math.max(value, min), max);
}

function getCenteredPan(viewport, scale, size) {
  return {
    x: Math.max((viewport.clientWidth - size.width * scale) / 2, 0),
    y: Math.max((viewport.clientHeight - size.height * scale) / 2, 0),
  };
}

function getFitState(viewport, size) {
  if (!viewport || !size.width || !size.height) {
    return null;
  }

  const nextScale = clamp(
    Math.min(
      (viewport.clientWidth - 32) / size.width,
      (viewport.clientHeight - 32) / size.height,
    ),
    MIN_SCALE,
    1.15,
  );

  return {
    scale: nextScale,
    pan: getCenteredPan(viewport, nextScale, size),
  };
}

export default function MermaidDiagram({ code, title, minHeight = 520 }) {
  const [svg, setSvg] = useState('');
  const [error, setError] = useState('');
  const [scale, setScale] = useState(1);
  const [pan, setPan] = useState({ x: 0, y: 0 });
  const [isDragging, setIsDragging] = useState(false);
  const diagramId = useId().replace(/[:]/g, '-');
  const viewportRef = useRef(null);
  const contentRef = useRef(null);
  const dragStateRef = useRef(null);
  const contentSizeRef = useRef({ width: 0, height: 0 });
  const scaleRef = useRef(1);
  const panRef = useRef({ x: 0, y: 0 });

  useEffect(() => {
    let active = true;

    const render = async () => {
      try {
        const mermaid = await getMermaid();
        const { svg: renderedSvg } = await mermaid.render(`diagram-${diagramId}`, code);

        if (!active) {
          return;
        }

        setSvg(renderedSvg);
        setError('');
      } catch (renderError) {
        if (!active) {
          return;
        }

        setError(renderError instanceof Error ? renderError.message : 'Mermaid render failed');
        setSvg('');
      }
    };

    render();

    return () => {
      active = false;
    };
  }, [code, diagramId]);

  useEffect(() => {
    if (!svg || !viewportRef.current || !contentRef.current) {
      return;
    }

    const svgElement = contentRef.current.querySelector('svg');
    if (!svgElement) {
      return;
    }

    const viewBox = svgElement.viewBox?.baseVal;
    const width =
      viewBox?.width ||
      Number.parseFloat(svgElement.getAttribute('width') || '') ||
      svgElement.getBBox?.().width ||
      svgElement.getBoundingClientRect().width;
    const height =
      viewBox?.height ||
      Number.parseFloat(svgElement.getAttribute('height') || '') ||
      svgElement.getBBox?.().height ||
      svgElement.getBoundingClientRect().height;

    contentSizeRef.current = {
      width: Number.isFinite(width) ? width : 0,
      height: Number.isFinite(height) ? height : 0,
    };

    const fitResult = getFitState(viewportRef.current, contentSizeRef.current);
    if (fitResult) {
      scaleRef.current = fitResult.scale;
      panRef.current = fitResult.pan;
      setScale(fitResult.scale);
      setPan(fitResult.pan);
    }
  }, [svg]);

  const updateViewport = useCallback((nextScale, nextPan) => {
    scaleRef.current = nextScale;
    panRef.current = nextPan;
    setScale(nextScale);
    setPan(nextPan);
  }, []);

  const zoomAtPoint = useCallback((nextScale, pointX, pointY) => {
    const boundedScale = clamp(nextScale, MIN_SCALE, MAX_SCALE);
    const currentScale = scaleRef.current || 1;
    const currentPan = panRef.current;
    const contentX = (pointX - currentPan.x) / currentScale;
    const contentY = (pointY - currentPan.y) / currentScale;

    updateViewport(boundedScale, {
      x: pointX - contentX * boundedScale,
      y: pointY - contentY * boundedScale,
    });
  }, [updateViewport]);

  const handleZoom = (direction) => {
    const viewport = viewportRef.current;
    if (!viewport) {
      return;
    }

    zoomAtPoint(
      scale + direction * ZOOM_STEP,
      viewport.clientWidth / 2,
      viewport.clientHeight / 2,
    );
  };

  const handleFit = () => {
    const fitResult = getFitState(viewportRef.current, contentSizeRef.current);
    if (!fitResult) {
      return;
    }

    updateViewport(fitResult.scale, fitResult.pan);
  };

  const handleReset = () => {
    const viewport = viewportRef.current;
    const { width, height } = contentSizeRef.current;
    if (!viewport || !width || !height) {
      return;
    }

    const nextScale = 1;
    updateViewport(nextScale, getCenteredPan(viewport, nextScale, contentSizeRef.current));
  };

  useEffect(() => {
    const viewport = viewportRef.current;
    if (!viewport) {
      return undefined;
    }

    const handleNativeWheel = (event) => {
      // React's synthetic wheel path is passive in Chromium-like browsers, so a
      // native non-passive listener is required to beat browser page zoom here.
      if (!event.ctrlKey && !event.metaKey) {
        return;
      }

      event.preventDefault();
      event.stopPropagation();

      const rect = viewport.getBoundingClientRect();
      const pointX = event.clientX - rect.left;
      const pointY = event.clientY - rect.top;
      const delta = event.deltaY < 0 ? ZOOM_STEP : -ZOOM_STEP;

      zoomAtPoint(scaleRef.current + delta, pointX, pointY);
    };

    viewport.addEventListener('wheel', handleNativeWheel, {
      passive: false,
      capture: true,
    });

    return () => {
      viewport.removeEventListener('wheel', handleNativeWheel, {
        capture: true,
      });
    };
  }, [zoomAtPoint]);

  const handlePointerDown = (event) => {
    if (event.button !== 0) {
      return;
    }

    dragStateRef.current = {
      startX: event.clientX,
      startY: event.clientY,
      panX: pan.x,
      panY: pan.y,
    };
    setIsDragging(true);
    event.currentTarget.setPointerCapture(event.pointerId);
  };

  const handlePointerMove = (event) => {
    if (!dragStateRef.current) {
      return;
    }

    const { startX, startY, panX, panY } = dragStateRef.current;
    updateViewport(scaleRef.current, {
      x: panX + (event.clientX - startX),
      y: panY + (event.clientY - startY),
    });
  };

  const handlePointerUp = (event) => {
    if (event.currentTarget.hasPointerCapture(event.pointerId)) {
      event.currentTarget.releasePointerCapture(event.pointerId);
    }

    dragStateRef.current = null;
    setIsDragging(false);
  };

  return (
    <div className="sg-mermaid-card">
      {title ? <div className="sg-mermaid-title">{title}</div> : null}
      {error ? (
        <div className="sg-alert">
          <p>{error}</p>
        </div>
      ) : (
        <>
          <div className="sg-mermaid-toolbar">
            <div className="sg-mermaid-actions">
              <button type="button" className="sg-mermaid-btn" onClick={() => handleZoom(-1)} aria-label="Zoom out">
                -
              </button>
              <button type="button" className="sg-mermaid-btn" onClick={() => handleZoom(1)} aria-label="Zoom in">
                +
              </button>
              <button type="button" className="sg-mermaid-btn" onClick={handleFit}>
                Fit
              </button>
              <button type="button" className="sg-mermaid-btn" onClick={handleReset}>
                1:1
              </button>
            </div>
            <div className="sg-mermaid-status">
              <span>{Math.round(scale * 100)}%</span>
              <span>Ctrl/Cmd + wheel to zoom, drag to move</span>
            </div>
          </div>

          <div
            ref={viewportRef}
            className={`sg-mermaid-canvas ${isDragging ? 'is-dragging' : ''}`}
            style={{ minHeight: typeof minHeight === 'number' ? `${minHeight}px` : minHeight }}
            onPointerDown={handlePointerDown}
            onPointerMove={handlePointerMove}
            onPointerUp={handlePointerUp}
            onPointerCancel={handlePointerUp}
          >
            <div
              ref={contentRef}
              className="sg-mermaid-stage"
              style={{
                transform: `translate(${pan.x}px, ${pan.y}px) scale(${scale})`,
              }}
              dangerouslySetInnerHTML={{ __html: svg }}
            />
          </div>
        </>
      )}
    </div>
  );
}
