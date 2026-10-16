# BREAK//174 — FM‑1 jungle instrument

[![Try it in browser](https://img.shields.io/badge/%E2%96%B6%20Try%20it%20in%20browser-BREAK%2F%2F174-00d7a0?style=for-the-badge)](https://eremansim.github.io/Fm1dnbchatgpt/)

Первая работающая alpha прошивки для **M‑VAVE FM‑1**: нажимаешь PLAY и получаешь готовый грув на 174 BPM. Выбираешь AMEN, держишь дробь или reverse, отпускаешь — основной ритм продолжает идти.

**Статус 0.1.0-alpha:** C-движок, WebAssembly-инструмент и пакет прошивки собираются; автоматические проверки проходят. **На физическом FM‑1 эта версия ещё не проверялась.** Это исходная точка для звуковой настройки и проверки реального времени, а не законченный концертный инструмент.

[Скачать .fwsc](releases/break174-0.1.0-alpha.fwsc) · [Управление](docs/CONTROLS.md) · [Сборка и установка](docs/BUILDING.md) · [Проверки и ограничения](docs/STATUS.md) · [Архитектура](docs/superpowers/specs/2026-10-16-break174-design.md)

## Уже играет

- Восемь процедурных барабанных голосов: kick, snare, ghost, closed/open hat, ride, tom, percussion. Оригинальные записи Amen и других брейков не используются.
- Семь семейств: Two Step, Amen Style, Think Style, Funky Ghost, Apache Style, Roller, Halftime; четырёхтактовая фраза, вариации с повторяемым seed.
- Профили DNB / Jungle / Liquid, четыре варианта тембра, саб и выбор его тоники в браузере.
- Сбивки, дроби 1/16–1/64, reverse, half-rate, gate, pitch down, dub throw с обратной связью, high-pass и drum kill.
- Внутренняя запись предыдущего такта в два PCM-буфера: 22.05 kHz mono. Единый музыкальный счётчик сохраняет позицию при исполнении эффектов.
- Восемь сцен, смена на границе такта, RETURN, USB MIDI-вход; настройки записываются после остановки и затихания.

Названия Amen / Think / Apache обозначают **авторские ритмические интерпретации**, а не точные реконструкции оригинальных записей. Liquid пока меняет характер ритма и движение саба; аккордового движка в этой alpha нет.

## Послушать до прошивки

В `web/` лежит самостоятельный инструмент с тем же C-движком через WASM. Готовый `break174.wasm` уже включён.

```sh
python3 -m http.server 8000 --directory web
```

Открой **http://localhost:8000**, нажми PLAY → белую AMEN → удерживай чёрные клавиши. SPACE запускает/останавливает, ESC возвращает сцену. REC WAV записывает до 60 секунд. HTTPS или localhost обязательны для AudioWorklet. Есть [12-секундное демо Amen](web/demos/amen-style.ogg) с дробью на 5-й секунде и reverse на 9-й.

## Устройство проекта

```text
core/        C99, fixed point, 44100 Hz, без malloc
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
python3 tests/test_package.py releases/break174-0.1.0-alpha.fwsc
```

Подробнее о target toolchain, SDK и установке — в [BUILDING](docs/BUILDING.md). Тесты не заменяют измерение загрузки CPU и прослушивание на FM‑1.

## Основа и лицензия

GPL‑3.0-only. Платформа взята из [zvenson/dinghy](https://github.com/zvenson/dinghy), commit `d44df7db09b98ccad2f1233b8f83f5f09c04829e`, на основе Felucca и SLOOP. Сохранены исходные уведомления об авторстве, механизм обновления и recovery. [Происхождение файлов](docs/upstream/PROVENANCE.md), [лицензирование платформы](docs/upstream/DINGHY-LICENSING.md).
