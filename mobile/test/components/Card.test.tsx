import { render, screen } from '@testing-library/react-native';
import { Text } from 'react-native';
import Card from '../../components/ui/Card';

describe('Card', () => {
  it('renders children', () => {
    render(<Card><Text>Hello</Text></Card>);
    expect(screen.getByText('Hello')).toBeTruthy();
  });

  it('renders title when provided', () => {
    render(<Card title="Board Info"><Text>Content</Text></Card>);
    expect(screen.getByText('Board Info')).toBeTruthy();
  });

  it('renders right slot', () => {
    render(<Card title="Test" right={<Text>Badge</Text>}><Text>Body</Text></Card>);
    expect(screen.getByText('Badge')).toBeTruthy();
  });

  it('renders warning text', () => {
    render(<Card title="Warn" warning="Danger!"><Text>Body</Text></Card>);
    expect(screen.getByText('Danger!')).toBeTruthy();
  });

  it('renders without title or right', () => {
    render(<Card><Text>Just content</Text></Card>);
    expect(screen.getByText('Just content')).toBeTruthy();
  });
});
