import { useState, useEffect } from 'react';
import ReactMarkdown from 'react-markdown';
import remarkGfm from 'remark-gfm';
import './DocsPage.css';

const DOC_SECTIONS = [
  { id: 'getting-started', title: 'Getting Started' },
  { id: 'hardware-setup', title: 'Hardware Setup' },
  { id: 'firmware-variants', title: 'Firmware Variants' },
  { id: 'commands', title: 'Command Reference' },
  { id: 'wifi-api', title: 'WiFi REST API' },
  { id: 'ble', title: 'Bluetooth (BLE)' },
  { id: 'can-protocol', title: 'CAN Protocol' },
  { id: 'vehicle-features', title: 'Vehicle Features' },
  { id: 'troubleshooting', title: 'Troubleshooting' },
  { id: 'e2e-test-plan', title: 'E2E Test Plan' },
  { id: 'can-review-checklist', title: 'CAN Review Checklist' },
];

export default function DocsPage() {
  const [activeDoc, setActiveDoc] = useState('getting-started');
  const [content, setContent] = useState('');
  const [loading, setLoading] = useState(true);

  useEffect(() => {
    setLoading(true); // eslint-disable-line react-hooks/set-state-in-effect -- data fetch loading state
    fetch(`/docs/${activeDoc}.md`)
      .then(r => {
        if (!r.ok) throw new Error('Not found');
        return r.text();
      })
      .then(text => { setContent(text); setLoading(false); })
      .catch(() => { setContent('# Not Found\n\nDocumentation file not available.'); setLoading(false); });
  }, [activeDoc]);

  // Handle internal doc links
  const handleLinkClick = (e: React.MouseEvent<HTMLElement>) => {
    const href = (e.target as HTMLElement)?.closest('a')?.getAttribute('href');
    if (!href) return;
    const match = DOC_SECTIONS.find(s => s.id === href);
    if (match) {
      e.preventDefault();
      setActiveDoc(match.id);
      window.scrollTo(0, 0);
    }
  };

  return (
    <div className="page page-docs">
      <div className="docs-layout">
        <nav className="docs-sidebar">
          <div className="docs-sidebar-title">Documentation</div>
          {DOC_SECTIONS.map(s => (
            <button
              key={s.id}
              className={`docs-sidebar-item ${activeDoc === s.id ? 'active' : ''}`}
              onClick={() => setActiveDoc(s.id)}
            >
              {s.title}
            </button>
          ))}
        </nav>
        <main className="docs-content" onClick={handleLinkClick}>
          {loading ? (
            <div className="docs-loading">Loading...</div>
          ) : (
            <div className="docs-prose">
              <ReactMarkdown remarkPlugins={[remarkGfm]}>{content}</ReactMarkdown>
            </div>
          )}
        </main>
      </div>
    </div>
  );
}
