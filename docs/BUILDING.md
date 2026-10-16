# Сборка и установка

## Прослушивание и тесты

Нужны Python 3, C99 compiler и Node 20+.

```sh
sh tests/run.sh
node tests/test_wasm.mjs
node tests/test_worklet.mjs
python3 tests/test_package.py releases/break174-0.1.0-alpha.fwsc
cc -std=c99 -O2 -Icore tools/render.c core/break174.c -o build/render
build/render build/amen.wav 12 1
python3 -m http.server 8000 --directory web
```

Параметр render после длительности — номер семейства 0–6. Для пересборки WASM установи официальный [wasi-sdk](https://github.com/WebAssembly/wasi-sdk/releases); проверена версия 25.0:

```sh
WASM_CC=/path/to/wasi-sdk/bin/clang python3 tools/build_web.py
```

## FM‑1 target

Проверенная среда: Linux x86_64, Python 3 + Pillow, JieLi Linux toolchain `20260730.1`, AC79NN SDK V1.2.1. На других ОС скрипт использует Docker.

```sh
python3 -m pip install Pillow
sh tools/get_toolchain.sh
```

SDK доступен в [официальном репозитории JieLi](https://gitee.com/Jieli-Tech/fw-AC79_AIoT_SDK). Нужны файлы относительно корня SDK:

- `cpu/wl82/tools/uboot.boot`
- `cpu/wl82/tools/cfg_tool.bin`
- `cpu/wl82/tools/cfg/eq_cfg_hw.bin`

Их проверенные SHA‑256 записаны в `tools/build.py`. Эти SDK-файлы не хранятся отдельно в репозитории. Происхождение встроенных в пакет компонентов описано в `docs/upstream/DINGHY-LICENSING.md`.

```sh
JIELI_TOOLCHAIN=/path/to/jieli-linux-toolchains-20260730.1 \
AC79_SDK=/path/to/fw-AC79_AIoT_SDK \
sh build.sh --release 0.1.0-alpha
```

Результат: `build/break174-0.1.0-alpha.fwsc`. Идентификатор этой сборки — `FM-1_9174010`; namespace выбран для проекта, регистрация в Firmware Hub ещё не выполнялась. Сборщик проверяет размещение RAM, pool, ROM-вызовы, размер образа и доступ к регистрам только из HAL.

## Установка на FM‑1

**Alpha не проверена на физическом устройстве.** Сначала прослушай web-версию; перед аппаратным тестом сохрани нужные настройки и подготовь известную рабочую прошивку для возврата. Код OTA/recovery унаследован от Dinghy; его наличие в сборке не подтверждает успешное восстановление на твоём экземпляре.

Проверить файл:

```sh
sha256sum -c releases/SHA256SUMS
python3 tests/test_package.py releases/break174-0.1.0-alpha.fwsc
```

Подключи FM‑1 по USB и закрой программы, занявшие его MIDI-порт:

```sh
python3 -m pip install mido python-rtmidi
python3 tools/fm1_install.py --info
python3 tools/fm1_install.py releases/break174-0.1.0-alpha.fwsc
```

Installer сначала показывает устройство и запросит подтверждение. Не используй `--force` для обхода ошибки формата. Он проверяет модель, структуру/CRC пакета, маркер своего loader и идентичность после перезапуска. Проверка формата выполнена автоматически; реальная USB-установка ещё не выполнялась.

Если устройство уже в update mode после прерванной установки этого пакета, повтори ту же команду. Для чужого loader installer намеренно откажется продолжать: заверши ту установку её штатным инструментом. Не отключай питание во время записи.

## CI и web-хостинг

GitHub Actions запускает native, ASan/UBSan, WASM/worklet и package-тесты. Сборка FM‑1 требует внешнего SDK и выполнялась отдельно; CI её не подменяет.

`web/` — статическая папка. Опциональный workflow **Deploy web preview** публикует её в GitHub Pages после включения Settings → Pages → Source: GitHub Actions и ручного запуска. Автоматическая публикация alpha в Firmware Hub не настроена.
