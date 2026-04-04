import Dashboard from '../components/Dashboard';

export default function DashboardPage({ board }) {
  return (
    <div className="page-view page-view--workspace">
      <Dashboard board={board} />
    </div>
  );
}
