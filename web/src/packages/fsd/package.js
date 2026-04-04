import { BoardCommands } from '../../lib/board/commands';

const PROFILE_LABELS = {
  0: 'Chill',
  1: 'Normal',
  2: 'Hurry',
  3: 'Max',
  4: 'Sloth',
};

export const FSD_PROFILE_OPTIONS = [
  { value: 0, label: 'Chill' },
  { value: 1, label: 'Normal' },
  { value: 2, label: 'Hurry' },
  { value: 3, label: 'Max' },
  { value: 4, label: 'Sloth' },
];

export const FSD_COMMANDS = {
  enable: BoardCommands.fsd(true),
  disable: BoardCommands.fsd(false),
  refresh: BoardCommands.status(),
  setOffset(offset) {
    return BoardCommands.offset(offset);
  },
  setIsaChime(nextEnabled) {
    return BoardCommands.isaChime(nextEnabled);
  },
  setEnabled(nextEnabled) {
    return BoardCommands.fsd(nextEnabled);
  },
  setSpeedProfile(profile) {
    return BoardCommands.profile(profile);
  },
};

export function createFsdState() {
  return {
    enabled: false,
    enabledLabel: 'OFF',
    speedProfile: null,
    speedProfileLabel: '—',
    speedOffset: 0,
    isaChimeEnabled: false,
    features: {
      speedOffset: false,
      isaSpeedChime: false,
    },
  };
}

export function deriveFsdState(statusMessage) {
  const speedProfile = Number.isFinite(Number(statusMessage?.sp)) ? Number(statusMessage.sp) : null;
  const enabled = Number(statusMessage?.fsd) === 1;
  const features = statusMessage?.features || {};

  return {
    enabled,
    enabledLabel: enabled ? 'ACTIVE' : 'OFF',
    speedProfile,
    speedProfileLabel: speedProfile == null ? '—' : (PROFILE_LABELS[speedProfile] || `Level ${speedProfile}`),
    speedOffset: Number(statusMessage?.offset) || 0,
    isaChimeEnabled: Number(statusMessage?.isaChime) === 1,
    features: {
      speedOffset: Number(features.speedOffset ?? 0) === 1,
      isaSpeedChime: Number(features.isaSpeedChime ?? 0) === 1,
    },
  };
}
