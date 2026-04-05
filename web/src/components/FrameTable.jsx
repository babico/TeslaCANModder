export default function FrameTable({ frames, frameCount, onClear }) {
  return (
    <div className="panel frame-panel">
      <div className="panel-header">
        <h3>Live CAN Frames</h3>
        <div className="panel-actions">
          <button className="btn btn-sm btn-ghost" onClick={onClear}>Clear</button>
          <span className="badge">{frameCount} frames</span>
        </div>
      </div>
      
      <div className="frame-table-wrap">
        {frames.length === 0 ? (
          <div className="empty-state">Connect and start streaming to see CAN frames</div>
        ) : (
          <>
            <div className="frame-table-header">
              <span>Time</span>
              <span>Dir</span>
              <span>ID</span>
              <span>Seq</span>
              <span>DLC</span>
              <span>Data</span>
            </div>
            <div className="frame-table-body">
              {frames.map((frame) => (
                <div key={frame.key} className="frame-row">
                  <span className="frame-time">{frame.ts}</span>
                  <span className={`frame-dir ${frame.dir}`}>{frame.dir.toUpperCase()}</span>
                  <span className="frame-id mono">0x{frame.id.toString(16).toUpperCase()}</span>
                  <span className="frame-seq mono">{frame.seq ?? '—'}</span>
                  <span className="frame-dlc">{frame.dlc}</span>
                  <span className="frame-data mono">{frame.data || '—'}</span>
                </div>
              ))}
            </div>
          </>
        )}
      </div>
    </div>
  );
}
