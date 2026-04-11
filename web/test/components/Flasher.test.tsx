import { describe, it, expect } from 'vitest';
import { render, screen } from '@testing-library/react';
import userEvent from '@testing-library/user-event';
import Flasher from '../../src/components/Flasher';

describe('Flasher', () => {
  it('renders board selector with Arduino and ESP32', () => {
    render(<Flasher />);
    expect(screen.getByText('Arduino Uno R3')).toBeInTheDocument();
    expect(screen.getByText('ESP32-S DevKit')).toBeInTheDocument();
  });

  it('renders Arduino connectivity options by default', () => {
    render(<Flasher />);
    expect(screen.getByText('USB Serial')).toBeInTheDocument();
    expect(screen.getByText('HC-05 Bluetooth')).toBeInTheDocument();
  });

  it('shows ESP32 connectivity when ESP32 board selected', async () => {
    render(<Flasher />);
    await userEvent.click(screen.getByText('ESP32-S DevKit'));
    expect(screen.getByText('WiFi')).toBeInTheDocument();
    expect(screen.getByText('BLE')).toBeInTheDocument();
  });

  it('resolves Arduino env to uno by default', () => {
    render(<Flasher />);
    const resolved = screen.getByText('Resolved environment:').querySelector('code');
    expect(resolved).toHaveTextContent('uno');
  });

  it('resolves Arduino env to uno_bt when BT toggled', async () => {
    render(<Flasher />);
    await userEvent.click(screen.getByText('HC-05 Bluetooth'));
    const resolved = screen.getByText('Resolved environment:').querySelector('code');
    expect(resolved).toHaveTextContent('uno_bt');
  });

  it('resolves ESP32 env based on connectivity toggles', async () => {
    render(<Flasher />);
    await userEvent.click(screen.getByText('ESP32-S DevKit'));

    const getResolved = () => screen.getByText('Resolved environment:').querySelector('code')!;
    expect(getResolved()).toHaveTextContent('esp32');

    await userEvent.click(screen.getByText('WiFi'));
    expect(getResolved()).toHaveTextContent('esp32_wifi');

    await userEvent.click(screen.getByText('BLE'));
    expect(getResolved()).toHaveTextContent('esp32_wifi_ble');

    await userEvent.click(screen.getByText('WiFi'));
    expect(getResolved()).toHaveTextContent('esp32_ble');
  });

  it('renders CAN bus selection with FSD locked on', () => {
    render(<Flasher />);
    expect(screen.getByText('CAN Buses')).toBeInTheDocument();
    const fsdBtn = screen.getByTitle('FSD bus is always active');
    expect(fsdBtn).toBeInTheDocument();
  });

  it('toggles vehicle bus on click', async () => {
    render(<Flasher />);
    const vehicleBtn = screen.getByText('Vehicle').closest('button')!;
    // Initially OFF
    expect(vehicleBtn.textContent).toContain('OFF');
    await userEvent.click(vehicleBtn);
    expect(vehicleBtn.textContent).toContain('ON');
  });

  it('renders Build & Flash section', () => {
    render(<Flasher />);
    expect(screen.getByText('Build & Download')).toBeInTheDocument();
    expect(screen.getByText('Flash via USB')).toBeInTheDocument();
  });

  it('renders PlatformIO CLI reference', () => {
    render(<Flasher />);
    expect(screen.getByText(/pio run/)).toBeInTheDocument();
  });

  it('USB Serial toggle is locked and cannot be turned off', async () => {
    render(<Flasher />);
    const serialBtns = screen.getAllByText('USB Serial');
    const serialBtn = serialBtns[0].closest('button')!;
    expect(serialBtn.textContent).toContain('ON');
    await userEvent.click(serialBtn);
    // Still ON after click
    expect(serialBtn.textContent).toContain('ON');
  });
});
