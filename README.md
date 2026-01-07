# Magnify

## Getting started

I had need for a screen magnifier, but all available screen magnifiers on 
Linux needed dozens of dependencies met in order to work, so I wrote my own, 
with the added benefit of exploring the Xlib library.

This resulted is Magnify, a lightweight screen magnifier that runs well on my 
current setup of Void Linux with i3.
It runs on a minimal X11 system with an X server that doesn’t coalesce motion 
events aggressively, so it won't eat up unneccessary CPU cycles.

## Installation

```bash
make
sudo make install
```

## Help

Run `make help` to get a list of individual make options.
Clang utils and Valgrind may have to be installed for debugging options to work.

## Usage

Move the mouse over the screen region to magnify, or use hjkl keys 
to move around. Press q to quit the application.

## License

Distributed under the MIT license. Read LICENSE.txt for detailed information.

