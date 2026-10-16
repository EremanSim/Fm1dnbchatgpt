import fs from 'node:fs';import vm from 'node:vm';import assert from 'node:assert/strict';
const module=await WebAssembly.compile(fs.readFileSync('web/break174.wasm'));
function make(rate){let Processor;class Base{constructor(){this.messages=[];this.port={postMessage:m=>this.messages.push(m)};}}
 const sandbox={AudioWorkletProcessor:Base,WebAssembly,Int32Array,Array,sampleRate:rate,registerProcessor:(_,c)=>Processor=c};vm.createContext(sandbox);vm.runInContext(fs.readFileSync('web/worklet.js','utf8'),sandbox);return new Processor({processorOptions:{module}});}
for(const rate of [44100,48000,96000]){
 const p=make(rate);p.port.onmessage({data:{type:'command',command:1,value:1}});let energy=0;
 const blocks=Math.floor(rate/128*3);for(let k=0;k<blocks;k++){const out=[new Float32Array(128),new Float32Array(128)];assert(p.process([], [out]));for(const v of out[0]){assert(Number.isFinite(v)&&Math.abs(v)<=1);energy+=Math.abs(v);}}
 assert(energy>100);assert(Math.abs(p.e.status(3)-3*44100)<512);
 p.port.onmessage({data:{type:'key',black:1,index:2,down:1}});assert(p.e.status(9)&4);
 p.port.onmessage({data:{type:'key',black:1,index:2,down:0}});assert.equal(p.e.status(9),0);
 p.port.onmessage({data:{type:'record',value:true}});p.process([],[[new Float32Array(128),new Float32Array(128)]]);assert(p.messages.some(m=>m.type==='audio'&&m.l.length===128));
 console.log(`AudioWorklet ${rate}: audio, clock, resampling, release and capture passed`);
}
