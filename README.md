# New-OS

Minimal real operating system kernel started from scratch.

Goal: A new OS where ordinary people can control the system with natural language commands (e.g. "create folder", "make a child folder").

## Current Status

- Minimal 64-bit higher-half kernel using the Limine boot protocol
- Boots in QEMU and draws a simple white diagonal line on the framebuffer
- Ready for the next steps: console, memory management, interrupts, etc.

## How to build and run

### Prerequisites
- A Linux or macOS system (or WSL2 on Windows)
- `make`, `gcc` (or clang) that can produce x86-64 ELF
- `xorriso` (for ISO creation)
- `qemu-system-x86_64`
- Git

### Quick start (recommended)

1. Clone the official Limine C template (it has a complete working build system):

```bash
git clone https://github.com/limine-bootloader/limine-c-template-x86-64.git temp-template
cp -r temp-template/* .
rm -rf temp-template
```

2. Replace `kernel/src/main.c` with the version in this repository.

3. Build and run:

```bash
make
make run
```

You should see a white diagonal line on a black screen. That means the kernel is alive.

## Project Vision

This is the foundation of a new operating system designed so that common people can write simple natural language commands instead of traditional shell commands.

Next milestones:
1. Proper console / text output
2. Memory management
3. Interrupts + keyboard
4. Simple filesystem
5. Natural language command interpreter as a core system service

## License

MIT (or change as you prefer)
