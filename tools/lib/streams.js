/** Helpers for bounded, awaited file-stream writes. */

import { once } from "node:events";

/** Write a chunk, waiting for `drain` when the stream buffer is full. */
export async function writeWithDrain(stream, chunk) {
	if (!stream.write(chunk)) {
		await once(stream, "drain");
	}
}

/** End a stream and resolve only after `finish`; reject on stream error. */
export async function endStream(stream, priorErrors = []) {
	let captured = priorErrors[0] ?? null;
	const onError = (error) => {
		if (!captured) captured = error;
	};
	stream.on("error", onError);
	try {
		if (captured) throw captured;
		stream.end();
		if (captured) throw captured;
		await once(stream, "finish");
		if (captured) throw captured;
	} finally {
		stream.off("error", onError);
	}
}
