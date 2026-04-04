import CanExplorer from '../components/CanExplorer';

export default function ExplorerPage({ board }) {
  return (
    <div className="page-view page-view--workspace">
      <CanExplorer board={board} />
    </div>
  );
}
