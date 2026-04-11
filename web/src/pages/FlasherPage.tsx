import Flasher from '../components/Flasher';

export default function FlasherPage() {
  return (
    <div className="page page-flasher">
      <div className="sg-hero">
        <h1>Firmware Flasher</h1>
        <p>Select your board and firmware variant, then flash via USB or download</p>
      </div>
      <Flasher />
    </div>
  );
}
