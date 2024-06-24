:: for first time using conan : conan profile detect --force

@echo off
:: Install dependencies using Conan
conan install . --output-folder=build --build=missing

:: Change to the build directory
cd build

:: Generate the Visual Studio solution and project files using CMake
cmake .. -G "Visual Studio 17 2022" -DCMAKE_TOOLCHAIN_FILE="conan_toolchain.cmake"

:: Build the project using the generated solution file
cmake --build . --config Release

:: Change back to the original directory
cd ..
