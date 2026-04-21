#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>

const int TRAIL_SIZE = 20;

void frameBufferSizeCallback(GLFWwindow* window, int width, int height);
unsigned int createShaderFromFile(GLenum type, char* name);
unsigned int createShaderProgram(int shaderCount, ...); 

int main(void) {
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  GLFWwindow* window = glfwCreateWindow(500, 500, "Cool Cursor", NULL, NULL);
  if (window == NULL) {
    printf("Failed to initialize GLFW window!\n");
    glfwTerminate();
    return 0;
  }
  glfwMakeContextCurrent(window);
  glfwSetFramebufferSizeCallback(window, frameBufferSizeCallback);

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    printf("Failed to initialize glad loader!\n");
    glfwTerminate();
    return 0;
  }

  glViewport(0, 0, 500, 500);

  unsigned int vert = createShaderFromFile(GL_VERTEX_SHADER, "main.vert");
  unsigned int frag = createShaderFromFile(GL_FRAGMENT_SHADER, "main.frag");
  unsigned int program = createShaderProgram(2, vert, frag);

  float trailVertices[TRAIL_SIZE];

  unsigned int VBO, VAO;
  glGenBuffers(1, &VBO);
  glGenVertexArrays(1, &VAO);

  glBindVertexArray(VAO);

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(trailVertices), trailVertices, GL_DYNAMIC_DRAW);

  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);

  double lastX = 0, lastY = 0;

  while (!glfwWindowShouldClose(window)) {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    double x, y;
    glfwGetCursorPos(window, &x, &y);

    int w, h;
    glfwGetWindowSize(window, &w, &h);

    float ndcX = (2.0f * x / w) - 1.0f;
    float ndcY = 1.0f - (2.0f * y / h);

    float vectorX = x - lastX;
    float vectorY = y - lastY;

    float length = sqrt(pow(vectorY, 2) + pow(-vectorX, 2));
     
    float resultX = vectorY / length;
    float resultY = -vectorX / length;

    const float trailWidth = 25.0f;

    trailVertices[0] = (2.0f * (x + (resultX * trailWidth)) / w) - 1.0f;
    trailVertices[1] = 1.0f - (2.0f * (y + (resultY * trailWidth)) / h);
    trailVertices[2] = (2.0f * (x - (resultX * trailWidth)) / w) - 1.0f;
    trailVertices[3] = 1.0f - (2.0f * (y - (resultY * trailWidth)) / h);

    glUseProgram(program);

    glBindVertexArray(VAO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(trailVertices), trailVertices);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, TRAIL_SIZE / 2);

    for (int i = TRAIL_SIZE - 1; i >= 0 + 4; i -= 4) {
      trailVertices[i - 3] = trailVertices[i - 7];
      trailVertices[i - 2] = trailVertices[i - 6];
      trailVertices[i - 1] = trailVertices[i - 5];
      trailVertices[i] = trailVertices[i - 4];
    }

    lastX = x;
    lastY = y;

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glDeleteProgram(program);

  glfwTerminate();
  return 0;
}

void frameBufferSizeCallback(GLFWwindow* window, int width, int height) {
  glViewport(0, 0, width, height);
}

unsigned int createShaderFromFile(GLenum type, char* name) {
  unsigned int shader = glCreateShader(type);

  FILE* shaderFile = fopen(name, "rb");

  if (shaderFile == NULL) {
    printf("Shader \"%s\" doesn't exist!", name);
    return 0;
  }

  fseek(shaderFile, 0, SEEK_END);
  long size = ftell(shaderFile) + 1;
  rewind(shaderFile);

  char buffer[size]; 
  buffer[size - 1] = '\0';

  unsigned int chN = 0;
  char ch;
  while ((ch = fgetc(shaderFile)) != EOF) {
    buffer[chN] = ch;
    chN++;
  }

  const char* constBuffer = buffer;
  glShaderSource(shader, 1, &constBuffer, NULL);
  glCompileShader(shader);

  int success;
  char infoLog[512];
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(shader, sizeof(infoLog), NULL, infoLog);
    printf("Failed to compile \"%s\", error:\n%s", name, infoLog);
  }

  fclose(shaderFile);
  return shader;
}

unsigned int createShaderProgram(int shaderCount, ...) {
  va_list valist;
  va_start(valist, shaderCount);

  unsigned int program = glCreateProgram();

  for (int i = 0; i < shaderCount; i++) {
    glAttachShader(program, va_arg(valist, unsigned int));
  }

  glLinkProgram(program);

  int success;
  char infoLog[512];
  glGetProgramiv(program, GL_LINK_STATUS, &success);
  if (!success) {
    glGetProgramInfoLog(program, 512, NULL, infoLog);
    printf("Failed to link shader program, error:\n%s", infoLog);
  }

  for (int i = 0; i < shaderCount; i++) {
    glDeleteShader(va_arg(valist, unsigned int));
  }

  va_end(valist);
  return program;
}
