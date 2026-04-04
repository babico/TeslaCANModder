import Flasher from '../components/Flasher';

export default function FlashPage({ board }) {
  return (
    <div className="page-view page-view--document">
      <Flasher board={board} />
    </div>
  );
}
