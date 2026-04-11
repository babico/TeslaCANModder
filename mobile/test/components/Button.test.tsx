import { render, screen, fireEvent } from '@testing-library/react-native';
import Button from '../../components/ui/Button';

describe('Button', () => {
  it('renders label', () => {
    render(<Button label="Click me" onPress={jest.fn()} />);
    expect(screen.getByText('Click me')).toBeTruthy();
  });

  it('calls onPress when pressed', () => {
    const onPress = jest.fn();
    render(<Button label="Go" onPress={onPress} />);
    fireEvent.press(screen.getByText('Go'));
    expect(onPress).toHaveBeenCalledTimes(1);
  });

  it('does not call onPress when disabled', () => {
    const onPress = jest.fn();
    render(<Button label="No" onPress={onPress} disabled />);
    fireEvent.press(screen.getByText('No'));
    expect(onPress).not.toHaveBeenCalled();
  });

  it('renders all variants without crashing', () => {
    const noop = jest.fn();
    const { unmount: u1 } = render(<Button label="A" onPress={noop} variant="primary" />);
    u1();
    const { unmount: u2 } = render(<Button label="B" onPress={noop} variant="secondary" />);
    u2();
    const { unmount: u3 } = render(<Button label="C" onPress={noop} variant="ghost" />);
    u3();
    const { unmount: u4 } = render(<Button label="D" onPress={noop} variant="danger" />);
    u4();
    expect(true).toBe(true);
  });
});
