# Sound Lab — FM1 Open DSP audition

Three isolated audio probes for deciding the next BREAK//174 sound direction.
These are native-host renders of the **open C components** of M-VAVE FM1 Open,
not a recording from FM-1 and not a new flashable firmware.

Pinned upstream: https://github.com/m-vave/fm1_open/tree/b178ac43c25f010c60a878056fb5331fe693fd62

- Percussion: M-VAVE's MIT DaisySP/Plaits drum adaptation.
- Bass: M-VAVE's MIT Open303 C adaptation, with restrained filter settings.
- Keys: M-VAVE's MSFA/Dexed-derived six-operator FM C implementation, soft-EP recipe.
- No original commercial break recordings, sample packs or song melodies.
- No proprietary `libfm1_engine.a`, native SDK runtime or firmware/boot files copied.

Original component headers and full upstream licensing notices are in `vendor/fm1_open/`.
`fm_presets.c` is an extracted standalone section of upstream `app/presets.c`, with its
original license retained. Other imported code is unchanged.

Build: `python3 tools/build_sound_lab.py` (C compiler, ffmpeg, numpy, scipy).
Audio is rendered at 44100 Hz; downloadable FLAC is lossless stereo PCM24.
Each complete clip receives one fixed gain chosen for RMS/headroom; velocity changes and internal dynamics are preserved. No compressor or reverb is applied.
Automated checks detect bad buffers and clipping, not whether the timbre is enjoyable.
