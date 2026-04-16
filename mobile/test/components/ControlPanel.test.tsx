import { render, screen, fireEvent } from '@testing-library/react-native';
import ControlPanel from '../../components/ControlPanel';
import type { BoardState } from '@teslacanmodder/protocol';

const baseState: BoardState = {
  variant: 'hw4',
  hardware: 'ESP32-S3',
  driver: 'TWAI',
  board: 'esp32',
  uptime: 65000,
  rate: 500,
  fsd: true,
  nag: false,
  profile: 2,
  profilePinned: true,
  offset: 5,
  offsetPinned: false,
  isaChime: false,
  summonInject: false,
  summonActive: false,
  nagKiller: false,
  precondition: false,
  trackMode: false,
  otaInProgress: false,
  txPaused: false,
  detectedHW: 0,
  bmsVoltage: 0,
  bmsCurrent: 0,
  bmsPower: 0,
  bmsSoc: 0,
  bmsTempMin: 0,
  bmsTempMax: 0,
  bmsWhPerKm: 0,
  hasBms: false,
  canOnline: true,
  standby: false,
  bus1: true,
  bus2: true,
  bus3: false,
  busFsd: true,
  busVehicle: true,
  busBody: false,
  streaming: false,
  frames: [],
  frameCount: 0,
  messages: [],
  features: {
    fsd: true,
    profile: true,
    nag: true,
    speedOffset: true,
    isaSpeedChime: true,
    summon: false,
  },
};

describe('ControlPanel', () => {
  it('renders board info stats', () => {
    render(<ControlPanel state={baseState} connected={true} onCommand={jest.fn()} />);
    expect(screen.getByText('ESP32-S3')).toBeTruthy();
    expect(screen.getByText('TWAI')).toBeTruthy();
  });

  it('shows Speed Profile section', () => {
    render(<ControlPanel state={baseState} connected={true} onCommand={jest.fn()} />);
    expect(screen.getByText('Speed Profile')).toBeTruthy();
  });

  it('sends profile command on button press', () => {
    const onCommand = jest.fn();
    render(<ControlPanel state={baseState} connected={true} onCommand={onCommand} />);
    fireEvent.press(screen.getByText('Chill'));
    expect(onCommand).toHaveBeenCalledWith('profile:0');
  });

  it('shows profile names', () => {
    render(<ControlPanel state={baseState} connected={true} onCommand={jest.fn()} />);
    expect(screen.getByText('Chill')).toBeTruthy();
    expect(screen.getByText('Normal')).toBeTruthy();
    expect(screen.getByText('Hurry')).toBeTruthy();
    expect(screen.getByText('Max')).toBeTruthy();
    expect(screen.getByText('Sloth')).toBeTruthy();
  });

  it('renders disconnected state without crashing', () => {
    render(<ControlPanel state={baseState} connected={false} onCommand={jest.fn()} />);
    expect(screen.getByText('Speed Profile')).toBeTruthy();
  });
});
