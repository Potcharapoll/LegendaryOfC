# Info

LegendaryOfC is a 2D top-down educational purpose game with a small story involved to make a game more interactive and attractive to learn. 
This game is made for learning C language and includes these topics of C language in the game:
- Data Types
- Keywords
- Syntax
- A few functions from the standard library (stdio.h and stdlib.h)

# Getting Started

### Prerequisite
- Cmake
- GCC Compiler
- Ninja

### How To Play
```bash
git clone --recursive https://github.com/Potcharapoll/LegendaryOfC && cd LegendaryOfC
cmake -DCMAKE_BUILD_TYPE:STRING=Release -G Ninja -B build -S .
ninja -C build
cd build && ./legendaryofc
```

### Control
| Key | Description |
| --- | --- |
| WASD | Move around |
| E | Interact |
| Escape | Open/Close Menu |
| Spacebar | Continue dialog |
| A/D | Move between page in Man pages |
