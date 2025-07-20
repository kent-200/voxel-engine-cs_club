#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


#include <iostream>
#include <learnopengl/shader_m.h>

#include "../components/Ecs.h"
#include "../components/PhysicsSystem.h"

#include "../components/utils.h"

#include "../components/Texture.h"
// #include "../terrain/Plains.h"
// #include "../terrain/Hills.h"
// #include "../terrain/Platform.h"

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
Coordinator gCoordinator;

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
    ourShader->use();
    ourShader->setInt("texture1", 0);   // set texture1 in shader to binded texture #0
    ourShader->setFloat("texWidth", (1.0f / (float) blockTextures.atlasCols));
    ourShader->setFloat("texHeight", (1.0f / (float) blockTextures.atlasRows));


    // optional: back face culling
    // glEnable(GL_CULL_FACE);   // Enable backface culling
	// glCullFace(GL_BACK);      // Cull back faces
	// glFrontFace(GL_CCW);   


    // DEFINE FPS STRING

    // define terrain generator
    // -----------------------------
    TerrainGenerator *terrainGenerator = new TerrainGenerator(Chunk::CHUNK_SIZE, 0);

    // Use custom terrain generator
    // TerrainGenerator * terrainGenerator = new HillsTerrainGenerator(Chunk::CHUNK_SIZE, 1337);

    // initialize coordinator
    chunkManager = new ChunkManager(4, 3, ourShader, terrainGenerator);
    gCoordinator.Init(chunkManager);

    // generate terrain
    gCoordinator.mChunkManager->pregenerateChunks();

    gCoordinator.RegisterComponent<Gravity>();
    gCoordinator.RegisterComponent<RigidBody>();
    gCoordinator.RegisterComponent<Transform>();

    auto physicsSystem = gCoordinator.RegisterSystem<PhysicsSystem>();

    Signature signature;
    signature.set(gCoordinator.GetComponentType<Gravity>());
    signature.set(gCoordinator.GetComponentType<RigidBody>());
    signature.set(gCoordinator.GetComponentType<Transform>());
    gCoordinator.SetSystemSignature<PhysicsSystem>(signature);

     std::vector<Entity> entities(MAX_ENTITIES);

    // create a dummy "player entity"
    entities[0] = gCoordinator.CreateEntity();
    gCoordinator.AddComponent(entities[0],
                              Gravity{glm::vec3(0.0f, -0.05f, 0.0f)});
    gCoordinator.AddComponent(
        entities[0],
        RigidBody{glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f)});

    // place the entity in front of us
    gCoordinator.AddComponent(
        entities[0], Transform{.position = glm::vec3(0.0f, 10.0f, -5.0f),
                               .rotation = glm::vec3(0.0f, 0.0f, 0.0f),
                               .scale = glm::vec3(1.0f, 1.0f, 1.0f)});

    auto player = entities[0];

    // render loop
    // -----------

    while (!glfwWindowShouldClose(window)) {
        // per-frame time logic
        // --------------------
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        physicsSystem->Update(deltaTime);

        imguiWrapper.start_render();

        // input
        // -----
        processInput(window, &cursorOn);

        // render
        // ------
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // update
        gCoordinator.mChunkManager->update(deltaTime, gCoordinator.mCamera);
        // get player deets
        RigidBody playerRB = gCoordinator.GetComponent<RigidBody>(player);
        Transform playerTrans = gCoordinator.GetComponent<Transform>(player);

        // render
        gCoordinator.mChunkManager->render(gCoordinator.mCamera);
        
        // TODO: render the "player" entity
        defaultShader->use();
        glUseProgram(0);


        // render ImGui windows
        imguiWrapper.render(cursorOn, gCoordinator, playerRB, playerTrans, deltaTime);
        

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
        gCoordinator.mCamera.cameraSpeedMultiplier * deltaTime);
    constexpr glm::vec3 up = glm::vec3(0.0, 1.0, 0.0);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        gCoordinator.mCamera.cameraPos +=
            cameraSpeed * gCoordinator.mCamera.cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        gCoordinator.mCamera.cameraPos -=
            cameraSpeed * gCoordinator.mCamera.cameraFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        gCoordinator.mCamera.cameraPos -=
            glm::normalize(glm::cross(gCoordinator.mCamera.cameraFront,
                                      gCoordinator.mCamera.cameraUp)) *
            cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        gCoordinator.mCamera.cameraPos +=
            glm::normalize(glm::cross(gCoordinator.mCamera.cameraFront,
                                      gCoordinator.mCamera.cameraUp)) *
            cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        gCoordinator.mCamera.cameraPos += up * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
        gCoordinator.mCamera.cameraPos -= up * cameraSpeed;
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
        gCoordinator.mChunkManager->genChunk =
            !gCoordinator.mChunkManager->genChunk;
        keyPressMap[GLFW_KEY_X] = false;
    }

    // TODO: a better way to do this?
    gCoordinator.mCamera.frustum =
        createFrustumFromCamera(gCoordinator.mCamera);
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

    if (gCoordinator.mCamera.firstMouse) {
        gCoordinator.mCamera.lastX = xpos;
        gCoordinator.mCamera.lastY = ypos;
        gCoordinator.mCamera.firstMouse = false;
    }

    float xoffset = xpos - gCoordinator.mCamera.lastX;
    float yoffset = gCoordinator.mCamera.lastY -
                    ypos; // reversed since y-coordinates go from bottom to top
    gCoordinator.mCamera.lastX = xpos;
    gCoordinator.mCamera.lastY = ypos;

    float sensitivity = 0.1f; // change this value to your liking
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    gCoordinator.mCamera.yaw += xoffset;
    gCoordinator.mCamera.pitch += yoffset;

    // make sure that when pitch is out of bounds, screen doesn't get
    // flipped
    if (gCoordinator.mCamera.pitch > 89.0f)
        gCoordinator.mCamera.pitch = 89.0f;
    if (gCoordinator.mCamera.pitch < -89.0f)
        gCoordinator.mCamera.pitch = -89.0f;

    glm::vec3 front;
    front.x = cos(glm::radians(gCoordinator.mCamera.yaw)) *
              cos(glm::radians(gCoordinator.mCamera.pitch));
    front.y = sin(glm::radians(gCoordinator.mCamera.pitch));
    front.z = sin(glm::radians(gCoordinator.mCamera.yaw)) *
              cos(glm::radians(gCoordinator.mCamera.pitch));

    gCoordinator.mCamera.cameraFront = glm::normalize(front);

    // Calculate the right vector
    gCoordinator.mCamera.cameraRight = glm::normalize(glm::cross(
        gCoordinator.mCamera.cameraFront, glm::vec3(0.0f, 1.0f, 0.0f)));

    // Calculate the up vector
    gCoordinator.mCamera.cameraUp = glm::normalize(glm::cross(
        gCoordinator.mCamera.cameraRight, gCoordinator.mCamera.cameraFront));

    // Calculate the left vector (opposite of right)
    gCoordinator.mCamera.cameraLeft = -gCoordinator.mCamera.cameraRight;

    // The top vector is the same as the up vector in this case
    gCoordinator.mCamera.cameraTop = gCoordinator.mCamera.cameraUp;

    gCoordinator.mCamera.frustum =
        createFrustumFromCamera(gCoordinator.mCamera);
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
    gCoordinator.mCamera.cameraSpeedMultiplier -= (float)yoffset;
    if (gCoordinator.mCamera.cameraSpeedMultiplier < 1.0f)
        gCoordinator.mCamera.cameraSpeedMultiplier = 1.0f;
    if (gCoordinator.mCamera.cameraSpeedMultiplier > 1000.0f)
        gCoordinator.mCamera.cameraSpeedMultiplier = 1000.0f;

    gCoordinator.mCamera.frustum =
        createFrustumFromCamera(gCoordinator.mCamera);
}
