import Flasher from '../components/Flasher';

export default function FlasherPage() {
  return (
    <div className="page page-flasher">
      <div className="page-header">
        <h1>Firmware Flasher</h1>
        <p>Select and flash firmware to your Arduino Uno</p>
      </div>
      <Flasher />
    </div>
  );
}
