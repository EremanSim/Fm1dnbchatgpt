#!/usr/bin/env python3
"""Build the same portable C engine as a small freestanding WebAssembly module."""
import os,shutil,subprocess
from pathlib import Path
root=Path(__file__).resolve().parents[1]
clang=os.environ.get('WASM_CC') or shutil.which('clang')
if not clang:raise SystemExit('Set WASM_CC to wasi-sdk/bin/clang, or install clang + lld.')
exports=['init','command','key','hit','status','render','state_size']
cmd=[clang,'--target=wasm32','-std=c99','-O2','-nostdlib','-fno-builtin','-Icore','core/break174.c','core/wasm.c','-Wl,--no-entry','-Wl,--export-memory','-Wl,--initial-memory=524288','-Wl,--max-memory=524288']
cmd += ['-Wl,--export='+name for name in exports]
subprocess.run(cmd+['-o','web/break174.wasm'],cwd=root,check=True)
print('WASM built:',(root/'web/break174.wasm').stat().st_size,'bytes')
