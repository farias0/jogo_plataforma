# Jogo de plataforma

## Observations

* The game should be ran from the `build` directory, one level above the project's base directory. Currently, it uses the assets from the source files. It should be looked how to distribute the assets, so it won't depend on the source code. It should also be looked how to use absolute, OS-agnostic filepaths, so the program can be ran from anywhere.

## Dependencies

### Raylib

[Github](https://github.com/raysan5/raylib)

For Linux, compile and install it as a shared library, or use a package manager. Example:

```
git clone https://github.com/raysan5/raylib.git raylib
cd raylib
mkdir build && cd build
cmake -DBUILD_SHARED_LIBS=ON ..     # REMEMBER TO INCLUDE SHARED LIBRARY DEPENDENCIES
make
sudo make install
```

**PROTIP**: To uninstall a lib installed with `make install` that doesn't contain a `make uninstall` rule (like raylib), create and install a .deb of it using `checkinstall`, and then uninstall it. ([StackOverflow](https://stackoverflow.com/questions/1439950/whats-the-opposite-of-make-install-i-e-how-do-you-uninstall-a-library-in-li))

I still haven't managed to properly set up the project on Windows. Meanwhile the game can be compiled and ran through WSL2 with no issues.

Eventually I'll have to figure out how to compile raylib alongside the game, so it can be developed natively on Windows, and the game distribution won't have raylib as a dependency.

