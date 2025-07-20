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
    void render(bool cursorOn, Coordinator & gCoordinator, 
                RigidBody & playerRB, Transform & playerTrans, float deltaTime);

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



void ImguiWrapper::render(bool cursorOn, Coordinator & gCoordinator, RigidBody & playerRB,
                          Transform & playerTrans, float deltaTime) {
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

    ImGui::Begin("Player");
    ImGui::Text("velocity: (%.2f, %.3f, %.3f)", playerRB.velocity.x,
                playerRB.velocity.y, playerRB.velocity.z);
    ImGui::Text("position: (%.2f, %.3f, %.3f)", playerTrans.position.x,
                playerTrans.position.y, playerTrans.position.z);
    ImGui::End();

    ImGui::Begin("Camera");
    ImGui::Text("fov: %.2f", gCoordinator.mCamera.fov);
    ImGui::Text("pos: (%.2f, %.3f, %.3f)", gCoordinator.mCamera.cameraPos.x,
                gCoordinator.mCamera.cameraPos.y,
                gCoordinator.mCamera.cameraPos.z);
    ImGui::Text("left: (%.2f, %.3f, %.3f)",
                gCoordinator.mCamera.cameraLeft.x,
                gCoordinator.mCamera.cameraLeft.y,
                gCoordinator.mCamera.cameraLeft.z);
    ImGui::Text("right: (%.2f, %.3f, %.3f)",
                gCoordinator.mCamera.cameraRight.x,
                gCoordinator.mCamera.cameraRight.y,
                gCoordinator.mCamera.cameraRight.z);
    ImGui::Text("up: (%.2f, %.3f, %.3f)", gCoordinator.mCamera.cameraUp.x,
                gCoordinator.mCamera.cameraUp.y,
                gCoordinator.mCamera.cameraUp.z);
    ImGui::Text("frustum:");
    ImGui::Text("left d = %.2f",
                gCoordinator.mCamera.frustum.planes[Frustum::FRUSTUM_LEFT]
                    .distance);
    ImGui::Text("right d = %.2f",
                gCoordinator.mCamera.frustum.planes[Frustum::FRUSTUM_RIGHT]
                    .distance);
    ImGui::Text(
        "left: n:(%.2f, %.3f, %.3f)\nright: n:(%.3f, %.3f, %.3f)",
        gCoordinator.mCamera.frustum.planes[Frustum::FRUSTUM_LEFT].normal.x,
        gCoordinator.mCamera.frustum.planes[Frustum::FRUSTUM_LEFT].normal.y,
        gCoordinator.mCamera.frustum.planes[Frustum::FRUSTUM_LEFT].normal.z,
        gCoordinator.mCamera.frustum.planes[Frustum::FRUSTUM_RIGHT]
            .normal.x,
        gCoordinator.mCamera.frustum.planes[Frustum::FRUSTUM_RIGHT]
            .normal.y,
        gCoordinator.mCamera.frustum.planes[Frustum::FRUSTUM_RIGHT]
            .normal.z);

    ImGui::Text(
        "near: n:(%.2f, %.3f, %.3f)\nfar: n:(%.3f, %.3f, %.3f)",
        gCoordinator.mCamera.frustum.planes[Frustum::FRUSTUM_NEAR].normal.x,
        gCoordinator.mCamera.frustum.planes[Frustum::FRUSTUM_NEAR].normal.y,
        gCoordinator.mCamera.frustum.planes[Frustum::FRUSTUM_NEAR].normal.z,
        gCoordinator.mCamera.frustum.planes[Frustum::FRUSTUM_FAR].normal.x,
        gCoordinator.mCamera.frustum.planes[Frustum::FRUSTUM_FAR].normal.y,
        gCoordinator.mCamera.frustum.planes[Frustum::FRUSTUM_FAR].normal.z);

    ImGui::Text(
        "bottom: n:(%.2f, %.3f, %.3f)\ntop: n:(%.3f, %.3f, %.3f)",
        gCoordinator.mCamera.frustum.planes[Frustum::FRUSTUM_BOTTOM]
            .normal.x,
        gCoordinator.mCamera.frustum.planes[Frustum::FRUSTUM_BOTTOM]
            .normal.y,
        gCoordinator.mCamera.frustum.planes[Frustum::FRUSTUM_BOTTOM]
            .normal.z,
        gCoordinator.mCamera.frustum.planes[Frustum::FRUSTUM_TOP].normal.x,
        gCoordinator.mCamera.frustum.planes[Frustum::FRUSTUM_TOP].normal.y,
        gCoordinator.mCamera.frustum.planes[Frustum::FRUSTUM_TOP].normal.z);
    ImGui::End();

    if (cursorOn) {
        ImGui::Begin("Debug Menu");
        // if(DEBUG){
        // 	ImGui::DragInt("tps", &g.ticksPerSecond, 1, 0, 1000);
        // }
        // Text that appears in the window
        ImGui::Checkbox("generate chunks",
                        &gCoordinator.mChunkManager->genChunk);
        ImGui::LabelText("##moveSpeedLabel", "Movement Speed");
        ImGui::SliderFloat("##moveSpeedSlider",
                            &gCoordinator.mCamera.cameraSpeedMultiplier,
                            1.0f, 1000.0f);
        ImGui::LabelText("##chunkGenDistanceLabel", "Chunk Gen Distance");
        ImGui::SliderInt(
            "##chunkGenDistanceSlider",
            (int *)&(gCoordinator.mChunkManager->chunkGenDistance), 1, 16);
        ImGui::LabelText("##renderDistanceLabel", "Render Distance");
        ImGui::SliderInt(
            "##renderDistanceSlider",
            (int *)&(gCoordinator.mChunkManager->chunkRenderDistance), 1,
            16);
        ImGui::LabelText("##zFarLabel", "zFar");
        ImGui::SliderFloat("##zFarSlider", &gCoordinator.mCamera.zFar, 1.0f,
                            2000.0f);
        ImGui::LabelText("##fovSliderLabel", "FOV");
        ImGui::SliderFloat("##fovSlider", &gCoordinator.mCamera.fov, 25.0f,
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