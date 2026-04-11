import { render, screen } from '@testing-library/react-native';
import Badge from '../../components/ui/Badge';

describe('Badge', () => {
  it('renders label text', () => {
    render(<Badge label="Online" />);
    expect(screen.getByText('Online')).toBeTruthy();
  });

  it('uses default variant when none specified', () => {
    const { getByText } = render(<Badge label="test" />);
    expect(getByText('test')).toBeTruthy();
  });

  it('renders with success variant', () => {
    const { getByText } = render(<Badge label="OK" variant="success" />);
    expect(getByText('OK')).toBeTruthy();
  });

  it('renders with error variant', () => {
    const { getByText } = render(<Badge label="Err" variant="error" />);
    expect(getByText('Err')).toBeTruthy();
  });
});
