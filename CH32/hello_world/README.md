<div align="center">

  <h1>CH32V003 Hello World</h1>
  <h3>VS Code + CMake — no MounRiver IDE required</h3>

  [![License][license-shield]][license-url]
  [![Platform][platform-shield]][platform-url]
  [![Language][language-shield]][language-url]

</div>

<!-- MARKDOWN LINKS & IMAGES -->
[license-shield]: https://img.shields.io/badge/license-MIT-9565F6
[license-url]: LICENSE
[platform-shield]: https://img.shields.io/badge/platform-Linux-431D93
[platform-url]: #-prerequisites
[language-shield]: https://img.shields.io/badge/language-C-281158
[language-url]: src/main.c

<div align="center">
<p>
    <a href="#-get-started-5-minutes">Get Started</a> •
    <a href="#-prerequisites">Prerequisites</a> •
    <a href="#-sdk">SDK</a> •
    <a href="#-flash">Flash</a> •
    <a href="#-debug">Debug</a> •
    <a href="#-keyboard-shortcuts">Shortcuts</a> •
    <a href="#-faq">FAQ</a> •
    <a href="#-license">License</a>
</p>
</div>

---

A minimal "hello world" for the WCH CH32V003 RISC-V microcontroller, built,
flashed, and debugged entirely from **VS Code + CMake + GDB/OpenOCD** — no
MounRiver Studio (WCH's Eclipse-based IDE) required.

The build is a plain, non-interactive `cmake` + `make` invocation with no IDE
dependency, so it drops straight into any CI/CD system unmodified — that's
the point of this repo. (This repo doesn't ship a CI pipeline itself; it's a
proof that the build *is* CI-compatible.)

## 🚀 Get Started (5 minutes)

```bash
# Point at your toolchain (see Prerequisites below)
export CH32_TOOLCHAIN_PATH=/path/to/RISC-V\ Embedded\ GCC15
export CH32_OPENOCD_PATH=/path/to/OpenOCD/OpenOCD

# Build
./build.sh
```

That's it — the WCH SDK is fetched automatically, no MounRiver IDE, no manual
project wizard. See [Prerequisites](#-prerequisites) for where to get the two
env vars' targets, and [Flash](#-flash) / [Debug](#-debug) to get it running
on real hardware.

## 🏗️ Prerequisites

**This project currently only builds/flashes/debugs on Linux.** Both the
toolchain and OpenOCD binaries below are Linux x86_64 builds.

1. **Toolchain**: the MounRiver "RISC-V Embedded GCC" toolchain
   (`riscv32-wch-elf-gcc` and friends).
   - **Linux**: download the standalone bundle directly:
     [`MRS_Toolchain_Linux_X64_V240.tar.xz`](https://file-oss.mounriver.com/tools/MRS_Toolchain_Linux_X64_V240.tar.xz?sign=bd81ca7110d26ecbd1aa491a602f044f&time=1a07d91bdab&from=49.36.219.191&resId=2030113772066086913)
     (~411MB). This is a signed, possibly time-limited download link — if it
     stops working, get a fresh one from
     [MounRiver's download page](https://www.mounriver.com/download) —
     publicly accessible, no login required.
   - **Windows/macOS**: MounRiver doesn't currently offer this standalone
     toolchain bundle for those platforms. Instead, install
     [MounRiver Studio](https://www.mounriver.com/download) (the full IDE)
     once, then locate the toolchain inside its installation
     directory — on Linux it's bundled under `<MRS install dir>/toolchain/`
     as a `RISC-V Embedded GCC*` folder; the Windows/macOS installer layout
     wasn't verified here, but is expected to be analogous — and point
     `CH32_TOOLCHAIN_PATH` at that folder instead.

   Extract it anywhere and set:
   ```bash
   export CH32_TOOLCHAIN_PATH=/path/to/RISC-V\ Embedded\ GCC15
   ```
   (the folder that directly contains `bin/riscv32-wch-elf-gcc`).

2. **OpenOCD** (for flashing/debugging only, not needed to build): from the
   same MounRiver toolchain bundle above (Linux), or from
   `<MRS install dir>/toolchain/OpenOCD/` (Windows/macOS, extracted from the
   IDE the same way as the toolchain). Mainline/apt OpenOCD will **not**
   work — flashing/debugging the WCH-Link probe depends on WCH's proprietary
   `wlinke` adapter driver and `wch-riscv.cfg`, which only ship in WCH's own
   OpenOCD fork. Set:
   ```bash
   export CH32_OPENOCD_PATH=/path/to/OpenOCD/OpenOCD
   ```
   (the folder that directly contains `bin/openocd` and `bin/wch-riscv.cfg`).

Add both `export` lines to your shell profile so VS Code inherits them too.

## 📦 SDK

WCH's CH32-SDK (EVT — Core/Peripheral/Startup sources) is **fetched
automatically** at CMake configure time directly from WCH's own domain
(`file.wch.cn`, the same `CH32V003EVT.ZIP` linked from WCH's official
download center), pinned by SHA256 hash — it is not vendored into this repo.
Those fetched files carry WCH's own usage notice (see [License](#-license)
below), separate from this project's own license.

## 🔨 Build

```bash
./build.sh
```
or equivalently:
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
```
Both are plain non-interactive commands — this is what makes the project
CI/CD-compatible without any extra setup: any CI runner that has
`CH32_TOOLCHAIN_PATH` set up can run the exact same two commands.

## ⚡ Flash

In VS Code: `Ctrl+Shift+P → Tasks: Run Task → flash` (this rebuilds first —
`flash` depends on `build`, `build` depends on `cmake-configure`).

Or manually:
```bash
"$CH32_OPENOCD_PATH/bin/openocd" -f "$CH32_OPENOCD_PATH/bin/wch-riscv.cfg" \
    -c "program build/CH32_HelloWorld.elf verify reset exit"
```

## 🐞 Debug

In VS Code: `Debug CH32 (OpenOCD + GDB)` launch config, **F5**. Its
`preLaunchTask` is set to `flash` — so F5 alone rebuilds, reflashes, *and*
starts the debug session, in that order, every time. This matters: debugging
an ELF whose symbols don't match what's actually sitting in the chip's flash
is a real footgun (GDB computes breakpoint/step addresses from the symbol
table it has loaded — if that doesn't match the running code, stepping can
corrupt execution instead of just failing loudly). F5 always debugs exactly
what it just flashed.

Sets breakpoints, steps, and reads variables via GDB talking to OpenOCD over
the single-wire SDI debug interface.

## ⌨️ Keyboard Shortcuts

VS Code's tasks are chained via `dependsOn`, so one shortcut can trigger the
whole pipeline behind it:

| Shortcut | Runs | What actually happens |
|---|---|---|
| `Ctrl+Shift+B` | `build` (the default build task) | `cmake-configure` → `build` — compiles only, no flash |
| `F5` | `Debug CH32 (OpenOCD + GDB)` | `cmake-configure` → `build` → `flash` → attach GDB — compile, flash, *and* start debugging |
| `Ctrl+Shift+P` → *Tasks: Run Task* → `flash` | `flash` | `cmake-configure` → `build` → `flash` — compile and flash, no debugger attached |

Two gotchas worth knowing, both hit while building this project:
- `Ctrl+Shift+B` only lists tasks in the `build` task group — `flash` is in
  the `test` group, so it deliberately won't show up there. Use
  `Ctrl+Shift+P` → *Tasks: Run Task* to see every task regardless of group.
- Only one OpenOCD process can hold the debug adapter/port `3333` at a time.
  If a previous `flash` or debug session didn't exit cleanly, a new one will
  fail with "Address already in use" — find and stop the stale process
  (`lsof -i :3333`, then `kill <pid>`) before retrying.

## ❓ FAQ

**Q: "Why not just use MounRiver Studio?"**

A: MounRiver Studio works fine, but it's an Eclipse-based IDE with settings
spread across project/workspace/global preference scopes that don't map
cleanly to what you'd expect, and its debug/toolchain configs are opaque XML
files off in workspace metadata rather than next to the project. This repo
shows the same build/flash/debug workflow using two small, readable text
files (`CMakeLists.txt` and `cmake/toolchain.cmake`) instead.

**Q: "Does this work on Windows or macOS?"**

A: Not yet verified — see [Prerequisites](#-prerequisites). The CMake/VS Code
project structure itself isn't Linux-specific, but the toolchain and OpenOCD
binaries currently used are Linux x86_64 builds.

**Q: "Is this really CI/CD compatible?"**

A: Yes — `./build.sh` (or the two `cmake` commands in [Build](#-build)) is
the entire build, non-interactively, with no IDE in the loop. Any CI runner
with `CH32_TOOLCHAIN_PATH` set can run it as-is. This repo doesn't ship a CI
workflow file itself; it's a proof that the build *is* CI-compatible, not a
pipeline to copy-paste.

**Q: "Why does the SDK get downloaded instead of committed to the repo?"**

A: WCH's CH32-SDK files carry their own usage notice (see
[License](#-license)), not an open-source license — fetching it at build
time avoids redistributing it under different terms than WCH intended.

## 📜 License

This project's own files (`CMakeLists.txt`, `cmake/`, `.vscode/`, `Link.ld`,
`build.sh`, `src/main.c`, this README) are MIT-licensed — see
[LICENSE](LICENSE).

`src/debug.c`, `src/ch32v00x_it.c`, `src/system_ch32v00x.c`, and
`src/ch32v00x_conf.h` are adapted from WCH's CH32V003 EVT templates and
retain WCH's own header:
```
Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
Attention: This software (modified or not) and binary are used for
microcontroller manufactured by Nanjing Qinheng Microelectronics.
```
The SDK sources fetched automatically from `file.wch.cn` at build time carry
the same WCH notice.
