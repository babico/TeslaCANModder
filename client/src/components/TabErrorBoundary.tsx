import { Component, type ReactNode } from "react";
import { View, Text } from "react-native";
import { Button } from "../ui/shadcn/button";

interface Props {
	tab: string;
	children: ReactNode;
}

interface State {
	error: Error | null;
}

export class TabErrorBoundary extends Component<Props, State> {
	state: State = { error: null };

	static getDerivedStateFromError(error: Error): State {
		return { error };
	}

	handleRetry = (): void => {
		this.setState({ error: null });
	};

	render(): ReactNode {
		if (this.state.error) {
			return (
				<View className="flex-1 items-center justify-center gap-4 p-8">
					<Text className="text-lg font-semibold text-destructive">
						{this.props.tab} tab crashed
					</Text>
					<Text className="text-sm text-muted-foreground text-center">
						{this.state.error.message}
					</Text>
					<Button label="Retry" variant="outline" onPress={this.handleRetry} />
				</View>
			);
		}
		return this.props.children;
	}
}
