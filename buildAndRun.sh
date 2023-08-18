mkdir build
pushd build
cmake ..
cmake --build . # TODO test it with Windows
./space_invaders
popd