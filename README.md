# Cpp-Rogue-TUI

A terminal-based roguelike inspired by Vampire Survivors, written in C++ with ncurses.

## Features

- Map loaded from file with walls and doors
- Bump-to-attack combat
- Simple enemy AI
- Save/load (press P)

## Requirements

- g++ (or any C++ compiler)
- ncurses

## Building

```bash
g++ -g src/main.cpp -o build/main -lncurses
```

## Running

```bash
./build/main
```

## Controls

| Key | Action |
|-----|--------|
| `W/A/S/D` or Arrow keys | Move |
| `P` | Save game |
| `Q` | Quit |
