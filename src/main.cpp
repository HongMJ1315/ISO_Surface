#include <iostream>
#include <algorithm>
#include <vector>
#include <utility>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iomanip>
#include <fstream>

#include "glsl.h"
#include "GLinclude.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "reader.h"
#include "iso_surface.h"
#include "MarchingCubesTables.hpp"
#include <future>

int width = 800;
int height = 600;

// -------------------- Camera 全域變數 --------------------
glm::vec3 cameraPos   = glm::vec3(256.0f, 256.0f, 800.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp    = glm::vec3(0.0f, 1.0f, 0.0f);
float yaw   = -90.0f;      // 初始指向 -Z（使用 -90 度）
float pitch = 0.0f;
float lastX = width / 2.0f;
float lastY = height / 2.0f;
bool firstMouse = true;

float deltaTime = 0.0f;
float lastFrame = 0.0f;
// --------------------------------------------------------

// 宣告一個 future 用來接收背景線程計算的結果（回傳兩個 surface）
std::future<std::pair<Surface, Surface>> surfaceFuture;

void processInput(GLFWwindow *window){
    float cameraSpeed = 50.f * deltaTime; 
    if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPos += cameraSpeed * cameraFront;
    if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos -= cameraSpeed * cameraFront;
    if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if(firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }
    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // 反轉 y 軸差值
    lastX = xpos;
    lastY = ypos;
    
    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;
    
    yaw   += xoffset;
    pitch += yoffset;
    
    if(pitch > 89.0f)
        pitch = 89.0f;
    if(pitch < -89.0f)
        pitch = -89.0f;
        
    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(front);
}

void reshape(GLFWwindow *window, int w, int h){
    width = w;
    height = h;
    glViewport(0, 0, width, height);
}

std::vector<unsigned char> data;
Iso_Surface iso_surface1, iso_surface2;

// 用來記錄每個 surface 的 VAO 與頂點數量
GLuint VAO1, VAO2;
GLsizei vertCount1, vertCount2;

std::vector<Surface> surfaces;

void init_data(){
    // 讀取資料並初始化兩個等值面
    read("Scalar/testing_engine.raw", "Scalar/testing_engine.inf", data);

    // iso_surface1: isovalue = 128, 紅色
    iso_surface1 = Iso_Surface(data, 256, 256, 256, glm::vec3(1.0f, 0.0f, 0.0f));
    iso_surface1.generate_cube(200.f);  // 假設你使用 Marching Cubes (generate_cube)
    surfaces.push_back({iso_surface1.getVertices(), iso_surface1.getNormals()});
    
    iso_surface1.generate_cube(10.f);
    surfaces.push_back({iso_surface1.getVertices(), iso_surface1.getNormals()});
}

// 建立一個函式，把某個 iso_surface 的頂點/法線存進 GPU (VAO, VBO)
void setupSurfaceVAO(Surface surface, GLuint& outVAO, GLsizei& outVertCount) {
    // 取得該 surface 的頂點與法線
    std::vector<glm::vec3> vertices = surface.vertices;
    std::vector<glm::vec3> normals  = surface.normals;
    outVertCount = static_cast<GLsizei>(vertices.size());

    // 產生 VAO, VBO
    GLuint VBO_pos, VBO_norm;
    glGenVertexArrays(1, &outVAO);
    glGenBuffers(1, &VBO_pos);
    glGenBuffers(1, &VBO_norm);

    glBindVertexArray(outVAO);

    // 頂點座標
    glBindBuffer(GL_ARRAY_BUFFER, VBO_pos);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), vertices.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);  // layout(location = 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);

    // 頂點法線
    glBindBuffer(GL_ARRAY_BUFFER, VBO_norm);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), normals.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(1);  // layout(location = 1)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);

    glBindVertexArray(0);

    // 注意：VBO_pos, VBO_norm 不要在這裡刪除，否則資料也被釋放
    // 可以把它們存到全域或 class 變數中維持有效
}

int main(int argc, char **argv){
    // 初始化
    glutInit(&argc, argv);
    if(!glfwInit()){
        return -1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow *window = glfwCreateWindow(width, height, "Hw1", nullptr, nullptr);
    if(!window){
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetWindowSizeCallback(window, reshape);

    // 設定滑鼠回呼，並隱藏游標（捕捉滑鼠）
    // glfwSetCursorPosCallback(window, mouse_callback);
    // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    glViewport(0, 0, width, height);
    glEnable(GL_DEPTH_TEST);

    std::cout << "GLFW version: " << glfwGetVersionString() << std::endl;
    const GLubyte* glslVersion = glGetString(GL_SHADING_LANGUAGE_VERSION);
    const GLubyte* renderer = glGetString(GL_RENDERER);
    const GLubyte* version = glGetString(GL_VERSION);
    std::cout << "GLSL version: " << glslVersion << std::endl;
    std::cout << "Renderer: " << renderer << std::endl;
    std::cout << "OpenGL version supported: " << version << std::endl;

    // ImGui 初始化
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 400");

    // 讀取資料並生成兩個 iso_surface
    init_data();

    // 分別設定兩個 iso_surface 的 VAO
    setupSurfaceVAO(surfaces[0], VAO1, vertCount1);
    setupSurfaceVAO(surfaces[1], VAO2, vertCount2);

    // 建立 shader
    int shaderProgram = setGLSLshaders("shader/phong.vert", "shader/phong.frag");

    // 模型矩陣（如有需要可再調整位置）
    glm::mat4 model = glm::mat4(1.0f);

    // 光源 & 顏色
    glm::vec3 lightPos(300.0f, 300.0f, 600.0f);
    glm::vec3 lightColor(1.0f, 1.0f, 1.0f);

    // 渲染主迴圈
    float ISO1 = 200.f, ISO2 = 10.f;
    while(!glfwWindowShouldClose(window)){
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);
        glfwPollEvents();

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // ImGui 畫面
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        {
            ImGui::Begin("Input Window");
            ImGui::Text("Camera Position: (%.2f, %.2f, %.2f)", cameraPos.x, cameraPos.y, cameraPos.z);
            ImGui::SliderFloat("ISO Value 1", &ISO1, 0.0f, 255.0f);
            ImGui::SliderFloat("ISO Value 2", &ISO2, 0.0f, 255.0f);
            // 當按下按鈕時，啟動背景線程計算新的等值面
            if(ImGui::Button("Render")){
                // 如果前一次的計算已完成（或未啟動）則啟動新計算
                if(!surfaceFuture.valid()){
                    // 捕捉當前 ISO 值
                    float iso1 = ISO1;
                    float iso2 = ISO2;
                    // 使用 std::async 非同步執行計算
                    surfaceFuture = std::async(std::launch::async, [iso1, iso2]() -> std::pair<Surface, Surface>{
                        // 注意：在背景線程中僅進行資料運算，不能呼叫 OpenGL API
                        Iso_Surface localSurfaceRed(data, 256, 256, 256, glm::vec3(1.0f, 0.0f, 0.0f));
                        localSurfaceRed.generate_cube(iso1);
                        Surface newRed = { localSurfaceRed.getVertices(), localSurfaceRed.getNormals() };

                        Iso_Surface localSurfaceGreen(data, 256, 256, 256, glm::vec3(0.0f, 1.0f, 0.0f));
                        localSurfaceGreen.generate_cube(iso2);
                        Surface newGreen = { localSurfaceGreen.getVertices(), localSurfaceGreen.getNormals() };

                        return std::make_pair(newRed, newGreen);
                    });
                }
            }
            ImGui::End();
        }

        // 如果背景計算完成，更新 surface 與 VAO（必須在主線程中進行 OpenGL 呼叫）
        if(surfaceFuture.valid() &&
            surfaceFuture.wait_for(std::chrono::seconds(0)) == std::future_status::ready){
            auto newSurfaces = surfaceFuture.get();
            surfaces[0] = newSurfaces.first;
            surfaces[1] = newSurfaces.second;
            // 更新 VAO，注意這裡會建立新的 VAO，請依需求管理舊資源
            setupSurfaceVAO(surfaces[0], VAO1, vertCount1);
            setupSurfaceVAO(surfaces[1], VAO2, vertCount2);
        }

        glUseProgram(shaderProgram);

        // 計算 view 與 projection 矩陣
        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float) width / height, 0.1f, 2000.0f);

        // 傳送矩陣給 shader
        GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
        GLint viewLoc = glGetUniformLocation(shaderProgram, "view");
        GLint projLoc = glGetUniformLocation(shaderProgram, "projection");
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

        GLint lightPosLoc = glGetUniformLocation(shaderProgram, "lightPos");
        GLint viewPosLoc = glGetUniformLocation(shaderProgram, "viewPos");
        GLint lightColorLoc = glGetUniformLocation(shaderProgram, "lightColor");
        GLint objectColorLoc = glGetUniformLocation(shaderProgram, "objectColor");
        glUniform3fv(lightPosLoc, 1, glm::value_ptr(lightPos));
        glUniform3fv(viewPosLoc, 1, glm::value_ptr(cameraPos));
        glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

        // 畫出第一個等值面 (紅色)
        glm::vec3 color1 = glm::vec3(1.0f, 0.0f, 0.0f);
        glUniform3fv(objectColorLoc, 1, glm::value_ptr(color1));
        glBindVertexArray(VAO1);
        glDrawArrays(GL_TRIANGLES, 0, vertCount1);
        glBindVertexArray(0);

        // 畫出第二個等值面 (綠色)
        glm::vec3 color2 = glm::vec3(0.0f, 1.0f, 0.0f);
        glUniform3fv(objectColorLoc, 1, glm::value_ptr(color2));
        glBindVertexArray(VAO2);
        glDrawArrays(GL_TRIANGLES, 0, vertCount2);
        glBindVertexArray(0);

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // 清理
    glDeleteVertexArrays(1, &VAO1);
    glDeleteVertexArrays(1, &VAO2);
    // 如果你有另外存 VBO_pos, VBO_norm，要在這裡也一起刪除

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
