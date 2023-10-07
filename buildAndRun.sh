set -e                  # exit on error
mkdir build || true     # ignore error
pushd build
cmake ..
cmake --build .
./jogo_plataforma
popd
