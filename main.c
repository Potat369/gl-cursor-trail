#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdarg.h>

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


  float vertices[] = {
     1.0f,  1.0f,  0.0f,
     1.0f, -1.0f,  0.0f,
    -1.0f, -1.0f,  0.0f,
    -1.0f,  1.0f,  0.0f,
  }; 

  int indices[] = {
    0, 1, 3,
    1, 2, 3
  };

  unsigned int VBO, VAO, EBO;
  glGenBuffers(1, &VBO);
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &EBO);

  glBindVertexArray(VAO);

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

  int valid = 0;
  float positions[30][2];
  
  while (!glfwWindowShouldClose(window)) {
    double x, y;   
    glfwGetCursorPos(window, &x, &y);

    int w, h;
    glfwGetWindowSize(window, &w, &h);

    if (valid < 30) {
      valid++;
    }
    for (int i = 1; i < valid; i++) {
      positions[i - 1][0] = positions[i][0];
      positions[i - 1][1] = positions[i][1];
    }
    positions[valid - 1][0] = x;
    positions[valid - 1][1] = h - y;

    glUseProgram(program);
    glUniform2fv(glGetUniformLocation(program, "positions"), 30, &positions[0][0]);
    glUniform1i(glGetUniformLocation(program, "valid"), valid);

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

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

  FILE* shaderFile = fopen(name, "r");

  if (shaderFile == NULL) {
    printf("Shader \"%s\" doesn't exist!", name);
    return 0;
  }

  fseek(shaderFile, 0, SEEK_END);
  long size = ftell(shaderFile) + 1;
  fseek(shaderFile, 0, SEEK_SET);

  char buffer[size]; 
  buffer[size - 1] = EOF;

  unsigned int chN = 0;
  char ch;
  while ((ch = fgetc(shaderFile)) != EOF) {
    buffer[chN] = ch;
    chN++;
  }

  const char* constBuffer = &(*buffer);
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
