#pragma once

void MainLoop();
int OpenWindow(int iWidth, int iHeight);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void getTexture();
void createShader();
void OpenGLInit();

GLFWwindow* window;
