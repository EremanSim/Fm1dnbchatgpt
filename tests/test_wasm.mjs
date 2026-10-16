import fs from 'node:fs';import assert from 'node:assert/strict';
const {instance}=await WebAssembly.instantiate(fs.readFileSync('web/break174.wasm'));
const e=instance.exports;e.init(174);assert.equal(e.status(1),166);e.command(1,1);
let energy=0;for(let i=0;i<350;i++){const p=e.render(256);for(const v of new Int32Array(e.memory.buffer,p,512))energy+=Math.abs(v);}
assert(energy>1e6);assert.equal(e.status(8),1);e.key(1,2,1);assert(e.status(9)&4);e.key(1,2,0);assert.equal(e.status(9),0);
e.command(16,0);e.render(256);assert.equal(e.status(2),0);assert(e.state_size()<310000);
console.log('WASM: exports, audio, print, release, panic and memory checks passed');
