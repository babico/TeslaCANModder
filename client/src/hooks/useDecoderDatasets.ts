import { useMemo, useState } from "react";
import { buildDecoderIndex, type DecoderDataset } from "@teslacanmodder/protocol";

import legacyMcu2Payload from "../assets/can-decoder/legacy_mcu2.json";
import legacyMcu3Payload from "../assets/can-decoder/legacy_mcu3.json";
import legacyModelSXIntelPayload from "../assets/can-decoder/legacy_modelsx_intel.json";
import legacyModelSXAmdPayload from "../assets/can-decoder/legacy_modelsx_amd.json";

type LegacyDecodedFrame = {
	address_dec?: number;
	frame_name?: string;
	signals?: Array<{
		signal_name?: string;
		possible_values?: Array<{ value_dec: number; value_hex: string; label: string }>;
	}>;
};

type LegacyDecodedPayload = {
	dataset_source?: { vehicle?: string; firmware?: string; mcu?: string; soc?: string };
	frames?: LegacyDecodedFrame[];
};

export type DecoderDatasetEntry = {
	id: string;
	label: string;
	dataset: DecoderDataset;
};

const LEGACY_DECODED_DATASET_SOURCES: Array<{
	id: string;
	label: string;
	payload: LegacyDecodedPayload;
}> = [
	{
		id: "legacy-mcu2",
		label: "Legacy Explorer MCU2",
		payload: legacyMcu2Payload as LegacyDecodedPayload,
	},
	{
		id: "legacy-mcu3",
		label: "Legacy Explorer MCU3",
		payload: legacyMcu3Payload as LegacyDecodedPayload,
	},
	{
		id: "legacy-modelsx-intel",
		label: "Legacy Explorer Model S/X Intel",
		payload: legacyModelSXIntelPayload as LegacyDecodedPayload,
	},
	{
		id: "legacy-modelsx-amd",
		label: "Legacy Explorer Model S/X AMD",
		payload: legacyModelSXAmdPayload as LegacyDecodedPayload,
	},
];

const DEFAULT_DECODER_DATASET_ID = LEGACY_DECODED_DATASET_SOURCES[0]?.id ?? "";

function coerceLegacyDecoderDataset(
	payload: LegacyDecodedPayload,
	fallbackLabel: string,
): DecoderDataset | null {
	if (!payload || !Array.isArray(payload.frames)) {
		return null;
	}

	const frames = payload.frames
		.filter((frame) => typeof frame.address_dec === "number")
		.map((frame) => ({
			id: Number(frame.address_dec),
			hex: `0x${Number(frame.address_dec).toString(16).toUpperCase()}`,
			frame_name:
				frame.frame_name ?? `Frame_${Number(frame.address_dec).toString(16).toUpperCase()}`,
			signals: Array.isArray(frame.signals)
				? frame.signals.map((signal) => ({
						signal_name: signal.signal_name ?? "UnknownSignal",
						possible_values: Array.isArray(signal.possible_values)
							? signal.possible_values
							: [],
					}))
				: [],
		}));

	if (frames.length === 0) {
		return null;
	}

	return {
		dataset_source: {
			vehicle: payload.dataset_source?.vehicle ?? "Tesla",
			firmware: payload.dataset_source?.firmware ?? fallbackLabel,
			mcu: payload.dataset_source?.mcu ?? "unknown",
			soc: payload.dataset_source?.soc ?? "unknown",
		},
		frames,
	};
}

export function useDecoderDatasets() {
	const [decoderDatasetId, setDecoderDatasetId] = useState<string>(DEFAULT_DECODER_DATASET_ID);

	const legacyDecoderDatasets = useMemo(
		() =>
			LEGACY_DECODED_DATASET_SOURCES.map((source) => {
				const dataset = coerceLegacyDecoderDataset(source.payload, source.label);
				if (!dataset) {
					return null;
				}
				return {
					id: source.id,
					label: source.label,
					dataset,
				} as DecoderDatasetEntry;
			}).filter((entry): entry is DecoderDatasetEntry => Boolean(entry)),
		[],
	);

	const allDecoderDatasets = useMemo(() => legacyDecoderDatasets, [legacyDecoderDatasets]);

	const selectedDecoderDataset = useMemo(
		() =>
			allDecoderDatasets.find((entry) => entry.id === decoderDatasetId) ??
			allDecoderDatasets[0] ?? {
				id: "none",
				label: "No Decoder Dataset",
				dataset: {
					dataset_source: {
						vehicle: "Tesla",
						firmware: "none",
						mcu: "none",
						soc: "none",
					},
					frames: [],
				},
			},
		[allDecoderDatasets, decoderDatasetId],
	);

	const decoderIndex = useMemo(
		() => buildDecoderIndex(selectedDecoderDataset.dataset),
		[selectedDecoderDataset],
	);

	return {
		allDecoderDatasets,
		selectedDecoderDataset,
		decoderIndex,
		decoderDatasetId,
		setDecoderDatasetId,
	};
}
