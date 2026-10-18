#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Build three original, isolated auditions using pinned open FM1 Open DSP."""
from pathlib import Path
import subprocess,os,json,hashlib
import numpy as np
from scipy.signal import butter,sosfilt
from scipy.io import wavfile
root=Path(__file__).resolve().parents[1];vendor=root/'sound_lab/vendor/fm1_open'
build=root/'build/sound-lab';out=root/'web/sound-lab';build.mkdir(parents=True,exist_ok=True);out.mkdir(parents=True,exist_ok=True)
sources=[vendor/n for n in ['drum808.c','bass303.c','fm_presets.c','fixed.c','adsr.c','dx7synth.c','dx7math.c','dx7note.c','dx7env.c','dx7lfo.c','dx7pitchenv.c','dx7core.c']]
subprocess.run([os.environ.get('CC','cc'),'-std=c11','-O2','-DFM1_HOST_TEST','-DFM1_OPEN_WEB','-I'+str(vendor),str(root/'sound_lab/render.c'),*map(str,sources),'-lm','-o',str(build/'render')],check=True)
clips=[]
for name,title in [('drums','01 · Барабаны'),('bass','02 · Бас'),('keys','03 · Клавиши')]:
    raw=build/(name+'.f32');subprocess.run([str(build/'render'),name,str(raw)],check=True)
    a=np.fromfile(raw,'<f4').astype(float).reshape(-1,2)
    assert len(a)==44100*26 and np.isfinite(a).all()
    # DC removal and smooth export boundaries, not tone replacement or reverb.
    a=sosfilt(butter(1,12,btype='highpass',fs=44100,output='sos'),a,axis=0)
    a[:441]*=np.linspace(0,1,441)[:,None];a[-4410:]*=np.linspace(1,0,4410)[:,None]
    rms=float(np.sqrt((a*a).mean()));peak=float(abs(a).max());assert rms>1e-6
    # One fixed scalar per complete clip retains quiet/loud hits and note envelopes.
    gain=min(.1/rms,.78/peak);a*=gain
    pcm=np.rint(np.clip(a,-1,1)*2147483392).astype('<i4');wav=build/(name+'.wav');wavfile.write(wav,44100,pcm)
    flac=out/(name+'.flac');subprocess.run(['ffmpeg','-hide_banner','-loglevel','error','-y','-i',str(wav),'-c:a','flac','-sample_fmt','s32','-bits_per_raw_sample','24',str(flac)],check=True)
    clips.append({'id':name,'title':title,'file':flac.name,'seconds':26,'sample_rate':44100,'bits':24,'peak':round(float(abs(a).max()),6),'rms':round(float(np.sqrt((a*a).mean())),6),'waveform':[round(float(abs(x).max())/.8,4) for x in np.array_split(a,96)],'gain_db':round(float(20*np.log10(gain)),3),'sha256':hashlib.sha256(flac.read_bytes()).hexdigest()})
    print(name,'peak',clips[-1]['peak'],'RMS',clips[-1]['rms'],'FLAC',flac.stat().st_size)
(out/'manifest.json').write_text(json.dumps({'version':'sound-lab-1','upstream':'b178ac43c25f010c60a878056fb5331fe693fd62','normalization':'one constant gain per complete clip, no compressor or reverb','clips':clips},ensure_ascii=False,indent=2)+'\n')
