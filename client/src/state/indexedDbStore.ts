export function hasIndexedDb(): boolean {
	return (
		typeof globalThis !== "undefined" &&
		typeof (globalThis as { indexedDB?: unknown }).indexedDB !== "undefined"
	);
}

function openIndexedDb(dbName: string, storeName: string): Promise<IDBDatabase> {
	return new Promise((resolve, reject) => {
		const indexedDb = (globalThis as { indexedDB?: IDBFactory }).indexedDB;
		if (!indexedDb) {
			reject(new Error("indexedDB unavailable"));
			return;
		}

		const request = indexedDb.open(dbName, 1);
		request.onupgradeneeded = () => {
			const db = request.result;
			if (!db.objectStoreNames.contains(storeName)) {
				db.createObjectStore(storeName);
			}
		};
		request.onsuccess = () => resolve(request.result);
		request.onerror = () => reject(request.error ?? new Error("failed to open indexedDB"));
	});
}

function requestResult<T>(request: IDBRequest<T>, errorMessage: string): Promise<T> {
	return new Promise((resolve, reject) => {
		request.onsuccess = () => resolve(request.result);
		request.onerror = () => reject(request.error ?? new Error(errorMessage));
	});
}

async function withStore<T>(
	dbName: string,
	storeName: string,
	mode: IDBTransactionMode,
	run: (store: IDBObjectStore) => Promise<T>,
): Promise<T> {
	const db = await openIndexedDb(dbName, storeName);
	try {
		const tx = db.transaction(storeName, mode);
		return await run(tx.objectStore(storeName));
	} finally {
		db.close();
	}
}

export function readIndexedDbRecord<T>(
	dbName: string,
	storeName: string,
	recordKey: IDBValidKey,
): Promise<T | undefined> {
	return withStore(dbName, storeName, "readonly", (store) =>
		requestResult<T | undefined>(
			store.get(recordKey) as IDBRequest<T | undefined>,
			"indexedDB read failed",
		),
	);
}

export function writeIndexedDbRecord<T>(
	dbName: string,
	storeName: string,
	recordKey: IDBValidKey,
	value: T,
): Promise<void> {
	return withStore(dbName, storeName, "readwrite", async (store) => {
		await requestResult(store.put(value, recordKey), "indexedDB write failed");
	});
}
