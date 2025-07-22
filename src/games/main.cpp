#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


#include <iostream>
#include <learnopengl/shader_m.h>


#include "../components/utils.h"

#include "../components/Texture.h"
#include "../terrain/Plains.h"
#include "../terrain/Hills.h"
#include "../terrain/Platform.h"

#include "../components/ImguiWrapper.h"


void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void imgui_mouse_callback(GLFWwindow *window, double xpos, double ypos);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
void processInput(GLFWwindow *window, bool *cursorOn);
// void renderText(Shader &shader, std::string text, float x, float y, float scale, glm::vec3 color);

// timing
float deltaTime = 0.0f; // time between current frame and last frame
float lastFrame = 0.0f;

// key press map
std::unordered_map<int, bool> keyPressMap;

// Terrain chunk manager
ChunkManager *chunkManager;
// Global coordinator
Camera camera;

// const int FPS_HISTORY_CAP = 5000;
// const int MEM_HISTORY_CAP = 5000;
// std::vector<float> fpsHistory;
// std::vector<float> memHistory;


int main() {
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow *window =
        glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "woksol", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(0);

    // imgui!!!
    // Setup Dear ImGui context
    ImguiWrapper imguiWrapper(window);
    imguiWrapper.init();
    

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // tell GLFW to capture our mouse
    bool cursorOn = false;
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // build and compile shader programs
    // ------------------------------------
    Shader *ourShader =
        new Shader("src/shaders/terrain.vert", "src/shaders/terrain.frag");
    Shader *defaultShader =
        new Shader("src/shaders/shader.vert", "src/shaders/shader.frag");

    // glm::vec3 pos = glm::vec3(0, 0, 0);
    // Chunk chunk = Chunk(pos, ourShader);
    // chunk.load();
    // chunk.setup();

    // load textures
    // -----------------------------
    Texture blockTextures("src/textures/terrain.png", 16, 16);
    blockTextures.bindTexture(0);
    blockTextures.setShaderVariables(*ourShader, 0);


    // optional: back face culling
    // glEnable(GL_CULL_FACE);   // Enable backface culling
	// glCullFace(GL_BACK);      // Cull back faces
	// glFrontFace(GL_CCW);   


    // DEFINE FPS STRING

    // define terrain generator
    // -----------------------------
    TerrainGenerator *terrainGenerator = new HillsTerrainGenerator(Chunk::CHUNK_SIZE, 1337);

    // Use custom terrain generator
    // TerrainGenerator * terrainGenerator = new HillsTerrainGenerator(Chunk::CHUNK_SIZE, 1337);

    // initialize coordinator
    chunkManager = new ChunkManager(4, 3, ourShader, terrainGenerator);
    //gCoordinator.Init(chunkManager);

    // generate terrain
    chunkManager->pregenerateChunks();



    // render loop
    // -----------

    while (!glfwWindowShouldClose(window)) {
        // per-frame time logic
        // --------------------
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        imguiWrapper.start_render();

        // input
        // -----
        processInput(window, &cursorOn);

        // render
        // ------
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // update
        chunkManager->update(deltaTime, camera);
        // get player deets
       // RigidBody playerRB = gCoordinator.GetComponent<RigidBody>(player);
      //  Transform playerTrans = gCoordinator.GetComponent<Transform>(player);

        // render
        chunkManager->render(camera);
        
        // TODO: render the "player" entity
        // defaultShader->use();
        // glUseProgram(0);
            
                
                


        // render ImGui windows
        imguiWrapper.render(cursorOn, camera, chunkManager, deltaTime);
        

        // Swap buffers and poll IO events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    // glDeleteVertexArrays(1, &VAO);
    // glDeleteBuffers(1, &VBO);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    imguiWrapper.~ImguiWrapper();
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this
// frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window, bool *cursorOn) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    float cameraSpeed = static_cast<float>(
        camera.cameraSpeedMultiplier * deltaTime);
    constexpr glm::vec3 up = glm::vec3(0.0, 1.0, 0.0);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.cameraPos +=
            cameraSpeed * camera.cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.cameraPos -=
            cameraSpeed * camera.cameraFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.cameraPos -=
            glm::normalize(glm::cross(camera.cameraFront,
                                      camera.cameraUp)) *
            cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.cameraPos +=
            glm::normalize(glm::cross(camera.cameraFront,
                                      camera.cameraUp)) *
            cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        camera.cameraPos += up * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
        camera.cameraPos -= up * cameraSpeed;
    }

    // insert into key press map
    if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS) {
        keyPressMap[GLFW_KEY_B] = true;
    }

    if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) {
        keyPressMap[GLFW_KEY_X] = true;
    }

    if (glfwGetKey(window, GLFW_KEY_B) == GLFW_RELEASE &&
        keyPressMap[GLFW_KEY_B]) {
        *cursorOn = !(*cursorOn);
        glfwSetCursorPosCallback(window, *cursorOn ? imgui_mouse_callback
                                                   : mouse_callback);
        glfwSetScrollCallback(window, *cursorOn ? imgui_mouse_callback
                                                : scroll_callback);
        glfwSetInputMode(window, GLFW_CURSOR,
                         *cursorOn ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
        keyPressMap[GLFW_KEY_B] = false;
    }

    if (glfwGetKey(window, GLFW_KEY_X) == GLFW_RELEASE &&
        keyPressMap[GLFW_KEY_X]) {
        chunkManager->genChunk =
            !chunkManager->genChunk;
        keyPressMap[GLFW_KEY_X] = false;
    }

    // TODO: a better way to do this?
    camera.frustum =
        createFrustumFromCamera(camera);
}

// glfw: whenever the window size changed (by OS or user resize) this callback
// function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    // make sure the viewport matches the new window dimensions; note that width
    // and height will be significantly larger than specified on retina
    // displays.
    glViewport(0, 0, width, height);
}



// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow *window, double xposIn, double yposIn) {
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (camera.firstMouse) {
        camera.lastX = xpos;
        camera.lastY = ypos;
        camera.firstMouse = false;
    }

    float xoffset = xpos - camera.lastX;
    float yoffset = camera.lastY -
                    ypos; // reversed since y-coordinates go from bottom to top
    camera.lastX = xpos;
    camera.lastY = ypos;

    float sensitivity = 0.1f; // change this value to your liking
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    camera.yaw += xoffset;
    camera.pitch += yoffset;

    // make sure that when pitch is out of bounds, screen doesn't get
    // flipped
    if (camera.pitch > 89.0f)
        camera.pitch = 89.0f;
    if (camera.pitch < -89.0f)
        camera.pitch = -89.0f;

    glm::vec3 front;
    front.x = cos(glm::radians(camera.yaw)) *
              cos(glm::radians(camera.pitch));
    front.y = sin(glm::radians(camera.pitch));
    front.z = sin(glm::radians(camera.yaw)) *
              cos(glm::radians(camera.pitch));

    camera.cameraFront = glm::normalize(front);

    // Calculate the right vector
    camera.cameraRight = glm::normalize(glm::cross(
        camera.cameraFront, glm::vec3(0.0f, 1.0f, 0.0f)));

    // Calculate the up vector
    camera.cameraUp = glm::normalize(glm::cross(
        camera.cameraRight, camera.cameraFront));

    // Calculate the left vector (opposite of right)
    camera.cameraLeft = -camera.cameraRight;

    // The top vector is the same as the up vector in this case
    camera.cameraTop = camera.cameraUp;

    camera.frustum =
        createFrustumFromCamera(camera);
}

void imgui_mouse_callback(GLFWwindow *window, double xposIn, double yposIn) {
    ImGuiIO io = ImGui::GetIO();
    double cursorX, cursorY;
    glfwGetCursorPos(window, &cursorX, &cursorY);
    ImGui_ImplGlfw_CursorPosCallback(window, cursorX, cursorY);
}


// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
    camera.cameraSpeedMultiplier -= (float)yoffset;
    if (camera.cameraSpeedMultiplier < 1.0f)
        camera.cameraSpeedMultiplier = 1.0f;
    if (camera.cameraSpeedMultiplier > 1000.0f)
        camera.cameraSpeedMultiplier = 1000.0f;

    camera.frustum =
        createFrustumFromCamera(camera);
}
