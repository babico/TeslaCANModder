import { useState } from 'react';

export default function Console({ messages, connected, onCommand, onClear }) {
  const [input, setInput] = useState('');

  const handleSend = () => {
    if (!input.trim() || !connected) return;
    onCommand(input.trim());
    setInput('');
  };

  const handleKeyDown = (e) => {
    if (e.key === 'Enter') handleSend();
  };

  return (
    <div className="panel console-panel">
      <div className="panel-header">
        <h3>Console</h3>
        <button className="btn btn-sm btn-ghost" onClick={onClear}>Clear</button>
      </div>

      <div className="console-body">
        {messages.length === 0 ? (
          <div className="empty-state">Board messages will appear here</div>
        ) : (
          messages.map((msg) => (
            <div key={msg.id} className={`console-line ${msg.type}`}>
              <span className="console-ts">{msg.ts}</span>
              <span className="console-text">{msg.text}</span>
            </div>
          ))
        )}
      </div>

      <div className="console-input">
        <input
          type="text"
          placeholder="Type command..."
          value={input}
          onChange={(e) => setInput(e.target.value)}
          onKeyDown={handleKeyDown}
          disabled={!connected}
        />
        <button className="btn btn-sm" disabled={!connected} onClick={handleSend}>
          Send
        </button>
      </div>
    </div>
  );
}
