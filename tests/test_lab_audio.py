"""Check decoded published FLAC audio, not just an intermediate renderer buffer."""
from pathlib import Path
import hashlib,json,subprocess
import numpy as np
from scipy.io import wavfile
root=Path(__file__).resolve().parents[1]
assert (root/'web/sound-lab/manifest.json').exists(), 'three-clip lab has not been built'
manifest=json.loads((root/'web/sound-lab/manifest.json').read_text())
assert {c['id'] for c in manifest['clips']}=={'drums','bass','keys'}
fingerprints=[]
for clip in manifest['clips']:
    path=root/'web/sound-lab'/clip['file']
    assert path.exists() and hashlib.sha256(path.read_bytes()).hexdigest()==clip['sha256']
    probe=json.loads(subprocess.check_output(['ffprobe','-v','error','-select_streams','a:0','-show_entries','stream=codec_name,sample_rate,channels,bits_per_raw_sample','-of','json',str(path)]))['streams'][0]
    assert probe['codec_name']=='flac' and probe['bits_per_raw_sample']=='24'
    assert probe['sample_rate']=='44100' and probe['channels']==2
    rate=44100
    decoded=subprocess.check_output(['ffmpeg','-v','error','-i',str(path),'-f','f32le','-acodec','pcm_f32le','-'])
    a=np.frombuffer(decoded,'<f4').astype(float).reshape(-1,2)
    assert len(a)==rate*26 and np.isfinite(a).all()
    assert .2<abs(a).max()<.9
    assert np.sqrt((a*a).mean())>.015
    assert np.sqrt((a[rate*4:rate*21]**2).mean())>.02, clip['id']+' phrase became silent'
    assert abs(a.mean())<.003
    assert abs(a[-128:]).max()<.001
    # Lossless FLAC must contain the intended PCM at 24-bit export precision.
    source_rate,source=wavfile.read(root/'build/sound-lab'/(clip['id']+'.wav'))
    assert source_rate==rate and source.shape==a.shape
    assert abs(source.astype(float)/2**31-a).max()<1.3e-7
    fingerprints.append(float(np.sum(abs(a[:rate*2]))))
assert len(set(fingerprints))==3
print('sound lab: decoded FLACs pass SHA256, PCM24 precision, audio, phrase, DC and boundary checks')
