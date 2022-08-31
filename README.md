# Tiny Renderer

I follow lessons from https://github.com/ssloy/tinyrenderer to understand 
the way OpenGL or another graphic API works.

I developed a small rasterizer by implementing the 3d graphics pipeline on CPU. 
My goals are learning the details of the steps on the pipeline and finding out 
the magic on the GPU side.

The project contains:
* Drawing lines and triangles (Bresenham’s Line Drawing Algorithm)
* Interpolation (Barycentric Coordinate System)
* Model loader (tiny-obj-loader)
* Texture sampler
* Vertex shader
* Fragment shader
* Depth test
* 3D transformations (model, view, perspective projection)
* Tangent space normal mapping
* OpenGL viewer (only texture rendering)

The project is developed using **C** programming language and **cglm** library for most of the calculations.
The viewer of the project is a very simple OpenGL application which renders a texture to a quad. 

Samples from Rendered Images:

![tangent_space_normal_mapping](https://user-images.githubusercontent.com/96859854/187677173-85010a99-75ec-4273-8a29-b5d4173dd622.png)
![diablo_tangent_space_normal_mapping](https://user-images.githubusercontent.com/96859854/187679749-cdf2a49a-48e2-4bee-a8bb-472824584b87.png)


## Compilation: Build Project for VS 2022
    rm -rf <build_folder>
    mkdir <build_folder>
    cmake -G "Visual Studio 17" -S . -B <build_folder>
    cmake --build <build_folder>
    
## Resources
[1] https://github.com/ssloy/tinyrenderer

[2] https://github.com/syoyo/tinyobjloader-c

[3] https://github.com/recp/cglm
