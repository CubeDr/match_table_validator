This is a [Next.js](https://nextjs.org) project bootstrapped with [`create-next-app`](https://nextjs.org/docs/app/api-reference/cli/create-next-app).

## To run:
- Run App: `npm run dev`
- Build WASM:
```
emcc wasm/*.cpp \
-o public/wasm/match_generator.js \
-s WASM=1 \
-s MODULARIZE=1 \
-s EXPORT_ES6=1 \
-s EXPORTED_RUNTIME_METHODS=ccall,cwrap \
-s ALLOW_MEMORY_GROWTH=1 \
--bind \
-O3 \
-s ENVIRONMENT=web
```