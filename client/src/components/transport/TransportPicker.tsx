import { Pressable, Text, View } from "react-native";
import type { MonitorTransportType } from "../../hardware/transportPresentation";
import { MONITOR_TRANSPORT_OPTIONS } from "../../hardware/transportPresentation";

const DISPLAY_TRANSPORT_OPTIONS = MONITOR_TRANSPORT_OPTIONS.filter(
	(option) => option.id !== "bluetooth-serial",
);

export interface TransportPickerProps {
	selected: MonitorTransportType;
	onChange: (type: MonitorTransportType) => void;
}

function normalize(type: MonitorTransportType): MonitorTransportType {
	return type === "bluetooth-serial" ? "serial" : type;
}

export function TransportPicker({ selected, onChange }: TransportPickerProps) {
	const visual = normalize(selected);

	return (
		<View className="flex-row flex-wrap gap-2">
			{DISPLAY_TRANSPORT_OPTIONS.map((option) => {
				const active = option.id === visual;
				return (
					<Pressable
						key={option.id}
						onPress={() => onChange(option.id)}
						className={`min-w-[156px] flex-1 gap-1 px-3 py-3 rounded-xl border ${
							active ? "bg-muted border-primary" : "bg-background border-border"
						}`}
					>
						<Text
							className={`text-sm font-semibold ${
								active ? "text-primary" : "text-foreground"
							}`}
						>
							{option.label}
						</Text>
						<Text className="text-xs text-muted-foreground leading-4">
							{option.detail}
						</Text>
					</Pressable>
				);
			})}
		</View>
	);
}
