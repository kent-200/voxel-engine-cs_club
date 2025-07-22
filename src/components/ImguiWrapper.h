#ifndef IMGUI_WRAPPER_H
#define IMGUI_WRAPPER_H

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "util.h"       // getMemoryUsage
#include "Ecs.h"        // Coordinator



class ImguiWrapper {
public:
    ImguiWrapper(GLFWwindow *window);
    ~ImguiWrapper();

    void init();
    void start_render();
    void render(bool cursorOn, Camera & camera,  ChunkManager * chunkManager,
                 float deltaTime);

private:
    GLFWwindow *window;
    char fpsStr[32] = "FPS: 0";
    char memStr[32];

    int frameCount = 0;
    float elapsedTime = 0.0f;
    float lastTime = 0.0f;

    int calculateFPS(float deltaTime);
    float calculateMemUsage();


};


ImguiWrapper::ImguiWrapper(GLFWwindow *window) {
    this->window = window;
}


void ImguiWrapper::init(){
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |=
        ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |=
        ImGuiConfigFlags_NavEnableGamepad; // Enable Gamepad Controls
    // io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // IF using Docking
    // Branch

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(
        window, true); // Second param install_callback=true will install GLFW
                       // callbacks and chain to existing ones.
    ImGui_ImplOpenGL3_Init(nullptr);
}


ImguiWrapper::~ImguiWrapper() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}


void ImguiWrapper::start_render() {
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}



void ImguiWrapper::render(bool cursorOn, Camera & camera, ChunkManager * chunkManager, float deltaTime) {
     // Calculate  FPS
    int fps = calculateFPS(deltaTime);
    float mem = calculateMemUsage();
    if (fps != -1) {
        std::sprintf(fpsStr, "FPS: %d", fps);
    }
    std::sprintf(memStr, "RAM: %f MB", mem / 1000000);

    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f), ImGuiCond_Always,
                            ImVec2(0.0f, 0.0f));
    ImGuiWindowFlags statsFlags =
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
    statsFlags |= ImGuiWindowFlags_NoMove;
    bool active = true;
    ImGui::Begin("Stats", &active, statsFlags);
    ImGui::Text("%s", fpsStr);
    ImGui::Text("%s", memStr);
    ImGui::Separator();
    // Ends the window
    ImGui::End();

    // ImGui::Begin("Player");
    // ImGui::Text("velocity: (%.2f, %.3f, %.3f)", playerRB.velocity.x,
    //             playerRB.velocity.y, playerRB.velocity.z);
    // ImGui::Text("position: (%.2f, %.3f, %.3f)", playerTrans.position.x,
    //             playerTrans.position.y, playerTrans.position.z);
    // ImGui::End();

    ImGui::Begin("Camera");
    ImGui::Text("fov: %.2f", camera.fov);
    ImGui::Text("pos: (%.2f, %.3f, %.3f)", camera.cameraPos.x,
                camera.cameraPos.y,
                camera.cameraPos.z);
    ImGui::Text("left: (%.2f, %.3f, %.3f)",
                camera.cameraLeft.x,
                camera.cameraLeft.y,
                camera.cameraLeft.z);
    ImGui::Text("right: (%.2f, %.3f, %.3f)",
                camera.cameraRight.x,
                camera.cameraRight.y,
                camera.cameraRight.z);
    ImGui::Text("up: (%.2f, %.3f, %.3f)", camera.cameraUp.x,
                camera.cameraUp.y,
                camera.cameraUp.z);
    ImGui::Text("frustum:");
    ImGui::Text("left d = %.2f",
                camera.frustum.planes[Frustum::FRUSTUM_LEFT]
                    .distance);
    ImGui::Text("right d = %.2f",
                camera.frustum.planes[Frustum::FRUSTUM_RIGHT]
                    .distance);
    ImGui::Text(
        "left: n:(%.2f, %.3f, %.3f)\nright: n:(%.3f, %.3f, %.3f)",
        camera.frustum.planes[Frustum::FRUSTUM_LEFT].normal.x,
        camera.frustum.planes[Frustum::FRUSTUM_LEFT].normal.y,
        camera.frustum.planes[Frustum::FRUSTUM_LEFT].normal.z,
        camera.frustum.planes[Frustum::FRUSTUM_RIGHT]
            .normal.x,
        camera.frustum.planes[Frustum::FRUSTUM_RIGHT]
            .normal.y,
        camera.frustum.planes[Frustum::FRUSTUM_RIGHT]
            .normal.z);

    ImGui::Text(
        "near: n:(%.2f, %.3f, %.3f)\nfar: n:(%.3f, %.3f, %.3f)",
        camera.frustum.planes[Frustum::FRUSTUM_NEAR].normal.x,
        camera.frustum.planes[Frustum::FRUSTUM_NEAR].normal.y,
        camera.frustum.planes[Frustum::FRUSTUM_NEAR].normal.z,
        camera.frustum.planes[Frustum::FRUSTUM_FAR].normal.x,
        camera.frustum.planes[Frustum::FRUSTUM_FAR].normal.y,
        camera.frustum.planes[Frustum::FRUSTUM_FAR].normal.z);

    ImGui::Text(
        "bottom: n:(%.2f, %.3f, %.3f)\ntop: n:(%.3f, %.3f, %.3f)",
        camera.frustum.planes[Frustum::FRUSTUM_BOTTOM]
            .normal.x,
        camera.frustum.planes[Frustum::FRUSTUM_BOTTOM]
            .normal.y,
        camera.frustum.planes[Frustum::FRUSTUM_BOTTOM]
            .normal.z,
        camera.frustum.planes[Frustum::FRUSTUM_TOP].normal.x,
        camera.frustum.planes[Frustum::FRUSTUM_TOP].normal.y,
        camera.frustum.planes[Frustum::FRUSTUM_TOP].normal.z);
    ImGui::End();

    if (cursorOn) {
        ImGui::Begin("Debug Menu");
        // if(DEBUG){
        // 	ImGui::DragInt("tps", &g.ticksPerSecond, 1, 0, 1000);
        // }
        // Text that appears in the window
        ImGui::Checkbox("generate chunks",
                        &chunkManager->genChunk);
        ImGui::LabelText("##moveSpeedLabel", "Movement Speed");
        ImGui::SliderFloat("##moveSpeedSlider",
                            &camera.cameraSpeedMultiplier,
                            1.0f, 1000.0f);
        ImGui::LabelText("##chunkGenDistanceLabel", "Chunk Gen Distance");
        ImGui::SliderInt(
            "##chunkGenDistanceSlider",
            (int *)&(chunkManager->chunkGenDistance), 1, 16);
        ImGui::LabelText("##renderDistanceLabel", "Render Distance");
        ImGui::SliderInt(
            "##renderDistanceSlider",
            (int *)&(chunkManager->chunkRenderDistance), 1,
            16);
        ImGui::LabelText("##zFarLabel", "zFar");
        ImGui::SliderFloat("##zFarSlider", &camera.zFar, 1.0f,
                            2000.0f);
        ImGui::LabelText("##fovSliderLabel", "FOV");
        ImGui::SliderFloat("##fovSlider", &camera.fov, 25.0f,
                            105.0f);
        // Slider that appears in the window
        // Ends the window
        ImGui::End();
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}





// Function to calculate and return the FPS as a string
int ImguiWrapper::calculateFPS(float deltaTime) {
    elapsedTime += deltaTime;
    frameCount++;

    if (elapsedTime - lastTime >= 1.0f) { // Update every second
        lastTime = elapsedTime;
        int fps = frameCount;
        frameCount = 0;
        return fps;
    }

    return -1.0;
}

// Function to calculate and return the RAM usage as a string
float ImguiWrapper::calculateMemUsage() {
    float memUsage = (float)getMemoryUsage();
    return memUsage;
}


#endif