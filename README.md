# BREAK//174 — FM‑1 jungle instrument

[![Try it in browser](https://img.shields.io/badge/%E2%96%B6%20Try%20it%20in%20browser-BREAK%2F%2F174-00d7a0?style=for-the-badge)](https://eremansim.github.io/Fm1dnbchatgpt/)

Первая работающая alpha прошивки для **M‑VAVE FM‑1**: нажимаешь PLAY и получаешь готовый грув на 166 BPM. Выбираешь AMEN, держишь дробь или reverse, отпускаешь — основной ритм продолжает идти.

**Статус 0.2.0-alpha:** C-движок, WebAssembly-инструмент и пакет прошивки собираются; автоматические проверки проходят. **На физическом FM‑1 эта версия ещё не проверялась.** Это исходная точка для звуковой настройки и проверки реального времени, а не законченный концертный инструмент.

[Скачать .fwsc](releases/break174-0.2.0-alpha.fwsc) · [Управление](docs/CONTROLS.md) · [Сборка и установка](docs/BUILDING.md) · [Проверки и ограничения](docs/STATUS.md) · [Архитектура](docs/superpowers/specs/2026-10-16-break174-design.md)

**[Новые изолированные аудиопробы FM1 Open →](https://eremansim.github.io/Fm1dnbchatgpt/sound-lab/)** — барабаны, бас и шестиоператорный FM, по 26 секунд. Это отдельный прототип звуковой основы; прошивка 0.2 пока не заменена. [Сборка, происхождение и лицензии](sound_lab/README.md).

## Что изменилось в звуке 0.2

Полностью заменена примитивная ударная модель: мембраны, корпус, пружины и металлические моды синтезируются при подготовке оригинального ROM; на устройстве он воспроизводится с интерполяцией. Добавлены поздние offbeat-удары, тихие ghost notes и басовые паузы. Стартовая сцена — 166 BPM, F minor, Amen Flow, Atmosphere 86. Референс направления — атмосферный jungle / Good Looking; чужих мелодий и записей здесь нет. [Обоснование и источники](docs/SOUND-0.2.md).

## Уже играет

- Восемь голосов оригинального процедурного PCM: kick, snare, ghost, closed/open hat, ride, tom, percussion. Оригинальные записи Amen и других брейков не используются.
- Семь семейств: Soul Two Step, Amen Flow, Brushed Think, Funky Ghost, Apache Jazz, Deep Roller, Halftime; четырёхтактовая фраза, вариации с повторяемым seed.
- Профили Deep / Jungle / Atmos, четыре варианта тембра, саб и выбор его тоники в браузере.
- Сбивки, дроби 1/16–1/64, reverse, half-rate, gate, pitch down, dub throw с обратной связью, high-pass и drum kill.
- Внутренняя запись предыдущего такта в два PCM-буфера: 22.05 kHz mono. Единый музыкальный счётчик сохраняет позицию при исполнении эффектов.
- Восемь сцен, смена на границе такта, RETURN, USB MIDI-вход; настройки записываются после остановки и затихания.

Названия Amen / Think / Apache обозначают **авторские ритмические интерпретации**, а не точные реконструкции оригинальных записей. В 0.2 добавлены мягкие аккорды min9 / maj9 / 6/9, стерео-пэды и FM-клавиши. ATMOSPHERE регулирует весь музыкальный слой; 0 оставляет барабаны и бас.

## Послушать до прошивки

В `web/` лежит самостоятельный инструмент с тем же C-движком через WASM. Готовый `break174.wasm` уже включён.

```sh
python3 -m http.server 8000 --directory web
```

Открой **http://localhost:8000**, нажми PLAY → белую AMEN → удерживай чёрные клавиши. SPACE запускает/останавливает, ESC возвращает сцену. REC WAV записывает до 60 секунд. HTTPS или localhost обязательны для AudioWorklet. Есть [24-секундное демо Atmos](web/demos/atmos-0.2.ogg) и [барабаны с басом без атмосферы](web/demos/drums-0.2.ogg).

## Устройство проекта

```text
core/        C99, fixed point, 44100 Hz, синтезированный ROM, без malloc
firmware/    FM‑1 adapter, HAL, USB, updater и recovery
web/         AudioWorklet, WASM и интерфейс для прослушивания
releases/    alpha-пакет и SHA-256
tests/      native, WASM, worklet, input и package проверки
tools/      сборка, WAV-рендер и USB MIDI installer
```

Native, браузер и FM‑1 используют один код синтеза. Браузер пересчитывает частоту вывода при 48/96 kHz, сохраняя темп 44.1 kHz-движка.

## Проверить

```sh
sh tests/run.sh
node tests/test_wasm.mjs
node tests/test_worklet.mjs
python3 tests/test_package.py releases/break174-0.2.0-alpha.fwsc FM-1_9174020
```

Подробнее о target toolchain, SDK и установке — в [BUILDING](docs/BUILDING.md). Тесты не заменяют измерение загрузки CPU и прослушивание на FM‑1.

## Основа и лицензия

GPL‑3.0-only. Платформа взята из [zvenson/dinghy](https://github.com/zvenson/dinghy), commit `d44df7db09b98ccad2f1233b8f83f5f09c04829e`, на основе Felucca и SLOOP. Сохранены исходные уведомления об авторстве, механизм обновления и recovery. [Происхождение файлов](docs/upstream/PROVENANCE.md), [лицензирование платформы](docs/upstream/DINGHY-LICENSING.md).
