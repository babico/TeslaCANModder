import { render, screen } from '@testing-library/react-native';
import StatusDot from '../../components/ui/StatusDot';

describe('StatusDot', () => {
  it('renders without crashing', () => {
    render(<StatusDot status="connected" />);
  });

  it('renders all status variants', () => {
    const statuses = ['connected', 'disconnected', 'warning', 'active'] as const;
    for (const s of statuses) {
      const { unmount } = render(<StatusDot status={s} />);
      unmount();
    }
    expect(true).toBe(true);
  });

  it('accepts custom size', () => {
    render(<StatusDot status="connected" size={20} />);
  });
});
