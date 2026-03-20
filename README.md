<div align="center">

<h1>ARM-PRos Operating System</h1>

[![License](https://img.shields.io/badge/License-MIT-yellow?style=for-the-badge)](LICENSE.TXT)
[![Version](https://img.shields.io/badge/version-0.1-blue?style=for-the-badge)](docs/changes/v0.1.txt)

[![GitHub stars](https://img.shields.io/github/stars/PRoX2011/ARM-PRos?style=flat-square)](https://github.com/PRoX2011/ARM-PRos/stargazers)
[![Last commit](https://img.shields.io/github/last-commit/PRoX2011/ARM-PRos?style=flat-square)](https://github.com/PRoX2011/ARM-PRos/commits)
[![Contributions Welcome](https://img.shields.io/badge/contributions-welcome-brightgreen?style=flat-square)](CONTRIBUTING.md)


**A minimalistic 64-bit operating system written in C and Assembly for ARMv8-A architecture**

</div>

## Overview

**ARM-PRos** is my experimental operating system project for the ARMv8-A architecture. It's practically bare bones right now, but if I find the strength to develop this project further, I'll try to turn it into something more. 

## Roadmap

- [x] UART Driver: Basic text output (puts, puthex, putc)
- [ ] UART Input
- [ ] Interrupt Controller (GIC)
- [ ] Keyboard Driver
- [ ] Timer Driver
- [ ] String Library (`strcmp`, `strlen`, `atoi`)
- [ ] Kernel Shell
- [ ] Physical Memory Manager
- [ ] PCI Scanning

## Building from Source

Install `aarch64-linux-gnu-gcc` compiler using your package manager, then run build script:

```bash
chmod +x build-linux.sh
./build-linux.sh
```

## Runing ARM-PRos

Install `qemu-system-aarch64` emulator then run this command:

```bash
./run-linux.sh
```

Or manually:

```bash
qemu-system-aarch64 \
    -M virt \
    -cpu cortex-a53 \
    -m 1024 \
    -kernel build/KERNEL.ELF \
    -display gtk \
    -serial vc \
    -monitor stdio
```

<div align="center">

**Made with ❤️ by PRoX**

</div>