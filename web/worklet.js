/* SPDX-License-Identifier: GPL-3.0-only */
class Break174Processor extends AudioWorkletProcessor {
 constructor(options){super();this.e=new WebAssembly.Instance(options.processorOptions.module).exports;this.e.init(174);this.buffer=null;this.at=256;this.phase=0;this.fl=0;this.fr=0;this.sl=0;this.sr=0;this.nl=0;this.nr=0;this.primed=false;this.counter=0;this.recording=false;this.port.onmessage=({data:m})=>{if(m.type==='command')this.e.command(m.command,m.value);else if(m.type==='key')this.e.key(m.black,m.index,m.down);else if(m.type==='hit')this.e.hit(m.role,m.velocity);else if(m.type==='record')this.recording=m.value;};}
 next(){if(this.at>=256){const p=this.e.render(256);if(!this.buffer)this.buffer=new Int32Array(this.e.memory.buffer,p,512);this.at=0;}this.nl=this.buffer[this.at*2]/32768;this.nr=this.buffer[this.at*2+1]/32768;this.at++;}
 process(inputs,outputs){const out=outputs[0];if(!out?.length)return true;if(!this.primed){this.next();this.fl=this.nl;this.fr=this.nr;this.next();this.sl=this.nl;this.sr=this.nr;this.primed=true;}
  const n=out[0].length;for(let i=0;i<n;i++){out[0][i]=this.fl+(this.sl-this.fl)*this.phase;if(out[1])out[1][i]=this.fr+(this.sr-this.fr)*this.phase;this.phase+=44100/sampleRate;while(this.phase>=1){this.phase--;this.fl=this.sl;this.fr=this.sr;this.next();this.sl=this.nl;this.sr=this.nr;}}
  if(this.recording){const l=out[0].slice(),r=(out[1]||out[0]).slice();this.port.postMessage({type:'audio',l,r},[l.buffer,r.buffer]);}
  if(++this.counter%20===0)this.port.postMessage({type:'status',values:Array.from({length:22},(_,i)=>this.e.status(i+1))});return true;
 }
}
registerProcessor('break174',Break174Processor);
