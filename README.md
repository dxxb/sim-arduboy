# Sim-Arduboy
Arduboy board implementation using simavr

## Setup dependencies

### OSX

Install homebrew by running:
``` ShellSession
> /usr/bin/ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"
```

Then install SDL2 and ELF libraries:
```
> brew install sdl2 libelf
```

### Windows (TBD)
### Linux (TBD)

### Clone, build and run

Clone repository and build:
``` ShellSession
> git clone --recursive https://github.com/dxxb/sim-arduboy.git
> cd sim-arduboy
> make
```

Run your .hex file:
``` ShellSession
> ./sim_arduboy filename.hex
```
