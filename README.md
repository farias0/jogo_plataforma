# Jogo de plataforma

## Observations

* The game should be ran from the `build` directory, one level above the project's base directory. Currently, it uses the assets from the source files. It should be looked how to distribute the assets, so it won't depend on the source code. It should also be looked how to use absolute, OS-agnostic filepaths, so the program can be ran from anywhere.

* I still haven't managed to properly set up the project on Windows. Meanwhile the game can be compiled and ran through WSL2 with no issues.

