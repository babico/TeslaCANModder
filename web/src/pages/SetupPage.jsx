import SetupGuide from '../components/SetupGuide';

export default function SetupPage({ board }) {
  return (
    <div className="page-view page-view--document">
      <SetupGuide board={board} />
    </div>
  );
}
