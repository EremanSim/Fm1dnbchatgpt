/* SPDX-License-Identifier: GPL-3.0-only */
const players=[...document.querySelectorAll('audio')];
for(const player of players)player.addEventListener('play',()=>{for(const other of players)if(other!==player)other.pause();});
fetch('manifest.json').then(r=>{if(!r.ok)throw Error('manifest');return r.json();}).then(m=>{for(const clip of m.clips){const host=document.getElementById('wave-'+clip.id);if(!host||!clip.waveform)continue;for(const peak of clip.waveform){const bar=document.createElement('i');bar.style.height=Math.max(2,peak*100)+'%';host.append(bar);}}}).catch(()=>{});
