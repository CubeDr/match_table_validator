let wasmModule = null;
let isLoadingWasm = true;
let isWasmError = false;

async function loadWasm() {
    try {
        const moduleFactory = (await import('./wasm/match_generator.js')).default;
        const instance = await moduleFactory();
        wasmModule = instance;
        isLoadingWasm = false;
        console.log("Worker: Wasm module loaded successfully.");
        self.postMessage({ type: 'WORKER_READY' });
    } catch (err) {
        isLoadingWasm = false;
        isWasmError = true;
        console.error("Worker: Error loading Wasm module:", err);
        self.postMessage({ type: 'WASM_LOAD_ERROR', payload: err instanceof Error ? err.message : String(err) });
    }
}

self.onmessage = (event) => {
    const { type, payload } = event.data;

    if (type === 'GENERATE_MATCHES') {
        console.log("Worker: Received GENERATE_MATCHES message.");
        if (isLoadingWasm) {
            console.warn("Worker: Wasm not loaded yet, ignoring generate request.");
            self.postMessage({ type: 'GENERATION_ERROR', payload: 'WASM module is still loading.' });
            return;
        }
        if (isWasmError || !wasmModule) {
            console.error("Worker: Wasm module failed to load or is unavailable.");
            self.postMessage({ type: 'GENERATION_ERROR', payload: 'WASM module failed to load.' });
            return;
        }

        const { players, courts, games } = payload;

        try {
            const startedAt = Date.now();
            const resultString = wasmModule.generateMatches(players, courts, games);
            const elapsed = Date.now() - startedAt;
            console.log('Worker: Generation took ' + (elapsed / 1000) + 's.');

            const result = JSON.parse(resultString);

            if (result.status === 'success') {
                self.postMessage({ type: 'GENERATION_SUCCESS', payload: result.result });
            } else {
                console.error("Worker: Generation failed in Wasm:", result.message);
                self.postMessage({ type: 'GENERATION_ERROR', payload: result.message || 'Unknown generation error' });
            }
        } catch (err) {
            console.error("Worker: Error calling Wasm function or parsing result:", err);
            self.postMessage({ type: 'GENERATION_ERROR', payload: err instanceof Error ? err.message : String(err) });
        }
    }
};

loadWasm();