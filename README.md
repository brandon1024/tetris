# Tetris
<img src="tetris-screenshot.png" align="left"/>
Yet another Tetris implementation, written in C with a Curses frontend.

## Building and Installing
By default, tetris is installed into your user's ~/bin directory. To install, from the project root run:
```
$ mkdir build
$ cd build
$ cmake ..
$ make install

$ ~/bin/tetris
```

For a global install, run from the project root:
```
$ mkdir build
$ cd build
$ cmake -DCMAKE_INSTALL_PREFIX=/usr/local ..
$ make install

$ tetris
```

## Usage
Want to play?
```
$ tetris
```