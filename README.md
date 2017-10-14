# Sim-Arduboy
Arduboy board implementation using simavr

## Setup dependencies

### OSX

Install homebrew by running:
``` ShellSession
> /usr/bin/ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"
```

Then install the simavr tap and install all simavr’s dependencies with:
``` ShellSession
> brew tap osx-cross/avr
> brew install --HEAD osx-cross/avr/simavr
```

And finally install the SDL2 libraries:
```
> brew install sdl2
```

### Windows (TBD)
### Linux (TBD)

### Clone, build and run

Clone repository and build:
``` ShellSession
> git clone --recursive https://github.com/dxxb/sim-arduboy.git
> make
```

Run your .hex file:
``` ShellSession
> ./sim_arduboy filename.hex
```
