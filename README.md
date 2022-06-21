# Tiny Renderer

I follow lessons from https://github.com/ssloy/tinyrenderer to understand 
the way OpenGL or another graphic API works.

## Compilation: Build Project for VS 2022
    rm -rf <build_folder>
    mkdir <build_folder>
    cmake -G "Visual Studio 17" -S . -B <build_folder>
    cmake --build <build_folder>