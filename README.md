# Sim-Arduboy
Arduboy board implementation using simavr

Follow the instruction for your operating system's _Dependencies setup_ section and then jump to the _Build_ section.

## Dependencies setup

### OSX

Install homebrew by running:

``` ShellSession
> /usr/bin/ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"
```

Then install SDL2 and ELF libraries:

```
> brew install sdl2 libelf
```

If avr-gcc cross-compiler is not installed on your system:

```
> brew tap osx-cross/avr
> brew install avr-gcc
```
 
### Windows (TBD)
### Linux (TBD)

### Ubuntu/Debian

``` ShellSession
> sudo apt-get install build-essential libelf-dev libsdl2-dev freeglut3-dev
```

## Build

### Command line

Clone repository:

``` ShellSession
> git clone --recursive https://github.com/dxxb/sim-arduboy.git
```

Build:

``` ShellSession
> cd sim-arduboy
> make
```

Run your .hex file:

``` ShellSession
> ./sim_arduboy filename.hex
```

### CMake (OSX)

Create XCode project files:

``` ShellSession
> cd sim-arduboy
> cmake -G Xcode ./cmake
```

Then open with XCode and build.