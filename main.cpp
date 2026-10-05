#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <iostream>
#include <vector>
#include <cmath>

#include "shader.h"
#include "geometry.h"


// ======================= GLOBALS & STATE =======================
unsigned int SCR_WIDTH = 1280;
unsigned int SCR_HEIGHT = 720;
glm::vec3 cameraPos = glm::vec3(0.0f, 15.0f, 110.0f); // Spawn further out for a full view
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

bool firstMouse = true; 
float yaw = -90.0f, pitch = 0.0f, lastX = 640.0f, lastY = 360.0f, fov = 45.0f;
float deltaTime = 0.0f, lastFrame = 0.0f;

// Interactive Elements
// Interactive Elements
bool turnAmbient = true, turnDiffuse = true, turnSpecular = true, turnAllLights = true, animationRunning = true, useTexture = true, isDayMode = false, turnOutsideLights = true;
float doorX = 0.0f, windowY = 0.0f, animTime = 0.0f;
float fanAngle = 0.0f, fanSpeedMult = 1.0f, conveyorBoxZ = 0.0f;
glm::vec3 carPos(10.0f, 0.0f, 10.0f); 
float carRotation = 0.0f, carWheelRot = 0.0f, forkY = 0.0f;

// Lighting Points
#define NR_POINT_LIGHTS 14
glm::vec3 pointLightPositions[NR_POINT_LIGHTS];

void setupCeilingLights() {
    // Distribute 12 lights evenly across the warehouse ceiling
    // Warehouse is roughly from X: -35 to 35, Z: -35 to 35, Ceiling Height: 19
    int index = 0;
    for (float x = -20.0f; x <= 20.0f; x += 20.0f) {
        for (float z = -25.0f; z <= 25.0f; z += 16.6f) {
            if (index < 12) {
                pointLightPositions[index] = glm::vec3(x, 18.5f, z);
                index++;
            }
        }
    }
    // Entrance lights (controlled by Key 4)
    pointLightPositions[12] = glm::vec3(-12.5f, 15.0f, 40.5f);
    pointLightPositions[13] = glm::vec3( 12.5f, 15.0f, 40.5f);
}

// ======================= TEXTURES =======================
unsigned int loadTexture(const char *path) {
    unsigned int tex; glGenTextures(1, &tex);
    int w, h, c; 
    stbi_set_flip_vertically_on_load(true);
    unsigned char *data = stbi_load(path, &w, &h, &c, 0);
    if (data) {
        GLenum format = (c == 1) ? GL_RED : ((c == 3) ? GL_RGB : GL_RGBA);
        glBindTexture(GL_TEXTURE_2D, tex);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexImage2D(GL_TEXTURE_2D, 0, format, w, h, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        stbi_image_free(data);
    } else {
        // Fallback procedural checkerboard
        const int tw = 64, th = 64; unsigned char texD[tw*th*3];
        for(int y=0;y<th;y++) {
            for(int x=0;x<tw;x++) { 
                bool wh = ((x/8)+(y/8))%2==0; 
                unsigned char p = wh ? 200:100; 
                texD[(y*tw+x)*3]=p; texD[(y*tw+x)*3+1]=p; texD[(y*tw+x)*3+2]=p; 
            }
        }
        glBindTexture(GL_TEXTURE_2D, tex); 
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, tw, th, 0, GL_RGB, GL_UNSIGNED_BYTE, texD);
        glGenerateMipmap(GL_TEXTURE_2D);
        stbi_image_free(data);
    } 
    return tex;
}

// ======================= RENDER HELPERS =======================
// Recursively computes exact branch structures demonstrating formal computer-science fractal theory constraint.
void drawFractalBranches(Shader& shader, glm::mat4 currMat, int depth) {
    if (depth == 0) {
        // Base Condition: spawn organic small leaf canopy cluster
        drawMesh(sphVAO, sphVerts, shader.ID, glm::scale(currMat, glm::vec3(1.2f, 1.2f, 1.2f)), glm::vec3(0.12f, 0.45f, 0.15f), 0.1f, 0, 1.0f, true); 
        return;
    }
    // Draw hierarchical segment bounds scaling down cleanly by geometric depth rules
    float height = 1.0f + depth * 0.7f;
    float thickness = 0.04f + depth * 0.05f;
    drawMesh(cylVAO, cylVerts, shader.ID, glm::scale(currMat, glm::vec3(thickness, height, thickness)), glm::vec3(0.2f, 0.12f, 0.05f), 0.5f, 0, 1.0f, false, glm::vec3(0.0f)); 
    
    glm::mat4 topMat = glm::translate(currMat, glm::vec3(0.0f, height * 0.85f, 0.0f)); // Structural connection boundary

    // 3 distinct organic rotational branch vectors propagating geometric layout cleanly
    glm::mat4 leftB = glm::rotate(topMat, glm::radians(38.0f), glm::vec3(0, 0, 1));
    leftB = glm::rotate(leftB, glm::radians(float(depth * 55)), glm::vec3(0, 1, 0));
    drawFractalBranches(shader, leftB, depth - 1);
    
    glm::mat4 rightB = glm::rotate(topMat, glm::radians(-35.0f), glm::vec3(0, 0, 1));
    rightB = glm::rotate(rightB, glm::radians(float(depth * -30)), glm::vec3(1, 0, 0));
    drawFractalBranches(shader, rightB, depth - 1);
    
    glm::mat4 fwdB = glm::rotate(topMat, glm::radians(30.0f), glm::vec3(1, 0, 0));
    fwdB = glm::rotate(fwdB, glm::radians(120.0f), glm::vec3(0, 1, 0));
    drawFractalBranches(shader, fwdB, depth - 1);
}

void drawTree(Shader& shader, glm::vec3 pos) {
    glm::mat4 m = glm::translate(glm::mat4(1.0f), pos);
    // 1. Explicitly draw the solid main Cylindrical Trunk base organically scaling upwards
    drawMesh(cylVAO, cylVerts, shader.ID, glm::scale(m, glm::vec3(0.5f, 5.0f, 0.5f)), glm::vec3(0.2f, 0.12f, 0.05f), 0.5f, 0, 1.0f, false, glm::vec3(0.0f)); 
    // 2. Map strictly highly dense recursion branching logic (Depth: 5 generates 243 intricate branch connections)
    glm::mat4 topTrunk = glm::translate(m, glm::vec3(0.0f, 4.8f, 0.0f));
    drawFractalBranches(shader, topTrunk, 5); 
}

// Cubic Bezier interpolation path solver (Spline Mathematics)
glm::vec3 getBezierPoint(float t, glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3) {
    float u = 1.0f - t;
    float tt = t * t;
    float uu = u * u;
    float uuu = uu * u;
    float ttt = tt * t;
    
    glm::vec3 p = uuu * p0;
    p += 3 * uu * t * p1;
    p += 3 * u * tt * p2;
    p += ttt * p3;
    return p;
}
void drawCloud(Shader& shader, glm::vec3 pos) {
    // Realistic distributed cloud using small clusters along a Bezier curve
    for(int i=0; i<7; i++) {
        float t = (float)i/6.0f;
        glm::vec3 p = getBezierPoint(t, glm::vec3(0), glm::vec3(2.5f, 1.2f, 0), glm::vec3(5.0f, -1.2f, 0), glm::vec3(7.5f, 0, 0));
        float sz = 2.2f + (float)sin(i*1.5f)*0.6f;
        drawMesh(sphVAO, sphVerts, shader.ID, glm::scale(glm::translate(glm::mat4(1.0f), pos + p), glm::vec3(sz, sz*0.5f, sz*0.8f)), glm::vec3(1.0f, 1.0f, 1.0f), 0.1f, 0, 0.9f, true, glm::vec3(0.1f));
    }
}

// ======================= INPUT =======================
void framebuffer_size_callback(GLFWwindow* window, int w, int h) { 
    SCR_WIDTH = w; SCR_HEIGHT = h; glViewport(0, 0, w, h); 
}
void mouse_callback(GLFWwindow* w, double x, double y) {
    float fx = (float)x, fy = (float)y; if(firstMouse) { lastX = fx; lastY = fy; firstMouse = false; }
    yaw += (fx - lastX) * 0.1f; pitch += (lastY - fy) * 0.1f; 
    lastX = fx; lastY = fy;
    if(pitch > 89.0f) pitch = 89.0f; if(pitch < -89.0f) pitch = -89.0f;
    cameraFront = glm::normalize(glm::vec3((float)cos(glm::radians(yaw))*(float)cos(glm::radians(pitch)), (float)sin(glm::radians(pitch)), (float)sin(glm::radians(yaw))*(float)cos(glm::radians(pitch))));
}
void scroll_callback(GLFWwindow* w, double x, double y) { 
    fov -= (float)y; if(fov<1) fov=1; if(fov>45) fov=45; 
}
void key_callback(GLFWwindow* w, int key, int sc, int action, int mods) {
    if(action == GLFW_PRESS) {
        if(key == GLFW_KEY_5) turnAmbient = !turnAmbient;
        if(key == GLFW_KEY_6) turnDiffuse = !turnDiffuse;
        if(key == GLFW_KEY_7) turnSpecular = !turnSpecular;
        if(key == GLFW_KEY_L) turnAllLights = !turnAllLights;
        if(key == GLFW_KEY_G) animationRunning = !animationRunning;
        if(key == GLFW_KEY_LEFT_BRACKET) fanSpeedMult -= 0.5f;
        if(key == GLFW_KEY_RIGHT_BRACKET) fanSpeedMult += 0.5f;
        if(key == GLFW_KEY_T) useTexture = !useTexture;
        if(key == GLFW_KEY_Y) isDayMode = !isDayMode;
        if(key == GLFW_KEY_4) turnOutsideLights = !turnOutsideLights;
    }
}
void processInput(GLFWwindow *w) {
    if(glfwGetKey(w, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(w, true);
    float cS = 15.0f * deltaTime;
    if(glfwGetKey(w, GLFW_KEY_W) == GLFW_PRESS) cameraPos += cS * cameraFront;
    if(glfwGetKey(w, GLFW_KEY_S) == GLFW_PRESS) cameraPos -= cS * cameraFront;
    if(glfwGetKey(w, GLFW_KEY_A) == GLFW_PRESS) cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cS;
    if(glfwGetKey(w, GLFW_KEY_D) == GLFW_PRESS) cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cS;
    if(glfwGetKey(w, GLFW_KEY_E) == GLFW_PRESS) cameraPos += cameraUp * cS;
    if(glfwGetKey(w, GLFW_KEY_R) == GLFW_PRESS) cameraPos -= cameraUp * cS;
    
    float doorS = 7.0f * deltaTime;
    if(glfwGetKey(w, GLFW_KEY_O) == GLFW_PRESS) doorX += doorS;
    if(glfwGetKey(w, GLFW_KEY_P) == GLFW_PRESS) doorX -= doorS;
    if(doorX > 24.0f) doorX = 24.0f; if(doorX < 0.0f) doorX = 0.0f;
    
    if(glfwGetKey(w, GLFW_KEY_U) == GLFW_PRESS) windowY += doorS;
    if(glfwGetKey(w, GLFW_KEY_J) == GLFW_PRESS) windowY -= doorS;
    if(windowY > 7.0f) windowY = 7.0f; if(windowY < 0.0f) windowY = 0.0f;
    
    // Forklift Control
    float carS = 12.0f * deltaTime, rotS = 60.0f * deltaTime;
    if(glfwGetKey(w, GLFW_KEY_UP) == GLFW_PRESS) { 
        carPos.x += carS * (float)sin(glm::radians(carRotation)); 
        carPos.z += carS * (float)cos(glm::radians(carRotation)); 
        carWheelRot += 150.0f * deltaTime;
    }
    if(glfwGetKey(w, GLFW_KEY_DOWN) == GLFW_PRESS) { 
        carPos.x -= carS * (float)sin(glm::radians(carRotation)); 
        carPos.z -= carS * (float)cos(glm::radians(carRotation)); 
        carWheelRot -= 150.0f * deltaTime;
    }
    if(glfwGetKey(w, GLFW_KEY_LEFT) == GLFW_PRESS) carRotation += rotS;
    if(glfwGetKey(w, GLFW_KEY_RIGHT) == GLFW_PRESS) carRotation -= rotS;
    if(glfwGetKey(w, GLFW_KEY_Z) == GLFW_PRESS) forkY += 5.0f * deltaTime;
    if(glfwGetKey(w, GLFW_KEY_X) == GLFW_PRESS) forkY -= 5.0f * deltaTime;
    if(forkY > 8.0f) forkY = 8.0f; if(forkY < 0.0f) forkY = 0.0f;
}

// ======================= MAIN PROGRAM =======================
int main() {
    glfwInit(); 
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); 
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3); 
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "3D Warehouse", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback); 
    glfwSetCursorPosCallback(window, mouse_callback); 
    glfwSetScrollCallback(window, scroll_callback); 
    glfwSetKeyCallback(window, key_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND); 
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    Shader sceneShader("vertexShader.vs", "fragmentShader.fs");
    setupGeom();
    setupCeilingLights();
    unsigned int wallTexOut = loadTexture("tiles_out_tex.jpg");
    unsigned int wallTexIn = loadTexture("tiles_in_tex.jpg");
    unsigned int floorTex = loadTexture("floor_tex.jpg");
    unsigned int grassTex = loadTexture("grass_tex.jpg");
    unsigned int roadTex = loadTexture("road_tex.jpg");
    unsigned int doorTex = loadTexture("door_tex.jpg");
    unsigned int nameTex = loadTexture("nameplate.png");

    while (!glfwWindowShouldClose(window)) {
        float cf = (float)glfwGetTime(); 
        deltaTime = cf - lastFrame; 
        lastFrame = cf;
        
        processInput(window);
        
        if(animationRunning) {
            animTime = fmod(cf, 10.0f); // 10 second loop counter
            fanAngle += 120.0f * fanSpeedMult * deltaTime;
            if(fanAngle > 360.0f) fanAngle -= 360.0f;
            
            
            conveyorBoxZ -= 4.0f * deltaTime; 
            if(conveyorBoxZ < -20.0f) conveyorBoxZ += 40.0f; 
        }

        if(isDayMode) glClearColor(0.5f, 0.7f, 0.95f, 1.0f); // Bright blue sky
        else glClearColor(0.05f, 0.1f, 0.15f, 1.0f); // Night sky
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 
        
        sceneShader.use();

        // Pass standard flags
        sceneShader.setBool("turnAmbient", turnAmbient);
        sceneShader.setBool("turnDiffuse", turnDiffuse);
        sceneShader.setBool("turnSpecular", turnSpecular);
        sceneShader.setBool("turnAll", turnAllLights);
        sceneShader.setVec3("viewPos", cameraPos);

        // 1. Directional Light (Sun/Moon)
        if(isDayMode) {
            sceneShader.setVec3("dirLight.direction", -0.4f, -1.0f, -0.4f);
            sceneShader.setVec3("dirLight.ambient", 0.3f, 0.3f, 0.3f);
            sceneShader.setVec3("dirLight.diffuse", 1.0f, 0.95f, 0.8f); // Bright Sunlight
            sceneShader.setVec3("dirLight.specular", 0.5f, 0.5f, 0.5f);
        } else {
            sceneShader.setVec3("dirLight.direction", -0.4f, -0.8f, 1.0f);
            sceneShader.setVec3("dirLight.ambient", 0.1f, 0.1f, 0.05f);
            sceneShader.setVec3("dirLight.diffuse", 0.4f, 0.35f, 0.2f);
            sceneShader.setVec3("dirLight.specular", 0.4f, 0.35f, 0.2f);
        }

        for (int i = 0; i < NR_POINT_LIGHTS; i++) {
            std::string number = std::to_string(i);
            sceneShader.setVec3("pointLights[" + number + "].position", pointLightPositions[i]);
            
            float diffValue = 0.0f;
            float lin = 0.022f;
            float quad = 0.0019f;
            int isExt = 0;
            if (i < 12) {
                // Interior Lights: Use lessened brightness (0.35f)
                diffValue = turnDiffuse ? 0.35f : 0.0f;
                isExt = 0;
            } else {
                // Outside Entrance Lights: Extra bright (2.0f) and longer range
                // Independent of turnDiffuse (Interior Key 6)
                diffValue = turnOutsideLights ? 2.0f : 0.0f;
                lin = 0.007f;
                quad = 0.0002f;
                isExt = 1;
            }

            sceneShader.setVec3("pointLights[" + number + "].ambient", 0.1f, 0.1f, 0.1f);
            sceneShader.setVec3("pointLights[" + number + "].diffuse", diffValue, diffValue, diffValue);
            sceneShader.setVec3("pointLights[" + number + "].specular", diffValue, diffValue, diffValue);
            
            sceneShader.setFloat("pointLights[" + number + "].constant", 1.0f);
            sceneShader.setFloat("pointLights[" + number + "].linear", lin);
            sceneShader.setFloat("pointLights[" + number + "].quadratic", quad);
            sceneShader.setInt("pointLights[" + number + "].isExterior", isExt);
        }

        // 3. Camera Flashlight (SpotLight - Disabled or kept subtle)
        sceneShader.setVec3("spotLight.position", cameraPos);
        sceneShader.setVec3("spotLight.direction", cameraFront);
        sceneShader.setVec3("spotLight.ambient", 0.0f, 0.0f, 0.0f);
        sceneShader.setVec3("spotLight.diffuse", 0.4f, 0.4f, 0.4f); // Dimmer flashlight
        sceneShader.setVec3("spotLight.specular", 0.5f, 0.5f, 0.5f);
        sceneShader.setFloat("spotLight.constant", 1.0f);
        sceneShader.setFloat("spotLight.linear", 0.045f);
        sceneShader.setFloat("spotLight.quadratic", 0.0075f);
        sceneShader.setFloat("spotLight.cutOff", (float)cos(glm::radians(12.5f)));
        sceneShader.setFloat("spotLight.outerCutOff", (float)cos(glm::radians(15.0f)));

        // The Flying Courier Drone (Satisfies Spline Curve constraint)
        float tDraw = (sin(glfwGetTime() * 0.4f) + 1.0f) / 2.0f; 
        glm::vec3 dp0(  0.0f,  6.0f,  30.0f), dp1( 25.0f, 15.0f,   0.0f), dp2(-25.0f, 10.0f, -20.0f), dp3(  0.0f, 18.0f, -30.0f); 

        glm::vec3 dronePos = getBezierPoint(tDraw, dp0, dp1, dp2, dp3);
        glm::vec3 dNext = getBezierPoint(glm::min(tDraw + 0.05f, 1.0f), dp0, dp1, dp2, dp3);
        if (tDraw > 0.95f) dNext = getBezierPoint(0.95f, dp0, dp1, dp2, dp3); 
        glm::vec3 dFwd = glm::normalize(dNext - dronePos);
        
        glm::mat4 drM = glm::translate(glm::mat4(1.0f), dronePos);
        drM = glm::rotate(drM, (float)atan2(dFwd.x, dFwd.z), glm::vec3(0,1,0));
        drawMesh(sphVAO, sphVerts, sceneShader.ID, glm::scale(drM, glm::vec3(1.2f, 0.4f, 1.2f)), glm::vec3(0.0f, 0.8f, 1.0f), 0.8f, 0, 1.0f, true);
        drawMesh(cubeVAO, 36, sceneShader.ID, glm::scale(glm::translate(drM, glm::vec3(0,-0.3f,0)), glm::vec3(0.5f, 0.3f, 0.5f)), glm::vec3(0.2f));

        // Camera matrices
        glm::mat4 proj = glm::perspective(glm::radians(fov), (float)SCR_WIDTH/SCR_HEIGHT, 0.1f, 300.0f);
        glm::mat4 view = glm::lookAt(cameraPos, cameraPos+cameraFront, cameraUp);
        sceneShader.setMat4("projection", proj);
        sceneShader.setMat4("view", view);

        // ======================= ENVIRONMENT (OUTSIDE) =======================
        sceneShader.setInt("material.ignoreLighting", 1); // Exterior components ignore interior toggles
        
        // Asphalt Road (Just front)
        drawMesh(cubeVAO, 36, sceneShader.ID, glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(0,-0.51f, 70.0f)), glm::vec3(30.0f, 1.0f, 80.0f)), glm::vec3(0.15f, 0.15f, 0.15f), 0.5f, useTexture ? roadTex : 0, 1.0f, false, glm::vec3(0.0f)); 
        // Grass surrounding
        drawMesh(cubeVAO, 36, sceneShader.ID, glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(-50.0f,-0.55f, 30.0f)), glm::vec3(70.0f, 1.0f, 160.0f)), glm::vec3(0.1f, 0.3f, 0.1f), 0.5f, useTexture ? grassTex : 0, 1.0f, false, glm::vec3(0.0f)); 
        drawMesh(cubeVAO, 36, sceneShader.ID, glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3( 50.0f,-0.55f, 30.0f)), glm::vec3(70.0f, 1.0f, 160.0f)), glm::vec3(0.1f, 0.3f, 0.1f), 0.5f, useTexture ? grassTex : 0, 1.0f, false, glm::vec3(0.0f)); 
        drawMesh(cubeVAO, 36, sceneShader.ID, glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f,-0.55f, -70.0f)), glm::vec3(170.0f, 1.0f, 40.0f)), glm::vec3(0.1f, 0.3f, 0.1f), 0.5f, useTexture ? grassTex : 0, 1.0f, false, glm::vec3(0.0f)); 

        // --- NEW SKY ELEMENTS ---
        // --- DAY/NIGHT SKY ELEMENTS ---
        if(isDayMode) {
            sceneShader.setInt("material.ignoreLighting", 2); // Celestial: Ignore all point lights
            // 1. The Sun (Bright Golden/White)
            glm::vec3 sunPos(-30.0f, 60.0f, -80.0f);
            drawMesh(sphVAO, sphVerts, sceneShader.ID, glm::scale(glm::translate(glm::mat4(1.0f), sunPos), glm::vec3(8.0f)), glm::vec3(1.0f, 1.0f, 0.9f), 1.0f, 0, 1.0f, true, glm::vec3(1.0f, 0.9f, 0.6f)); 
            // Solar Halo
            glDepthMask(GL_FALSE);
            drawMesh(sphVAO, sphVerts, sceneShader.ID, glm::scale(glm::translate(glm::mat4(1.0f), sunPos), glm::vec3(15.0f)), glm::vec3(1.0f, 0.9f, 0.4f), 1.0f, 0, 0.2f, true, glm::vec3(0.4f, 0.3f, 0.1f));
            glDepthMask(GL_TRUE);

            // 2. Distributed Realistic Clouds
            drawCloud(sceneShader, glm::vec3(-45.0f, 45.0f, -60.0f));
            drawCloud(sceneShader, glm::vec3(  5.0f, 50.0f, -75.0f));
            drawCloud(sceneShader, glm::vec3( 35.0f, 42.0f, -50.0f));
            drawCloud(sceneShader, glm::vec3(-20.0f, 48.0f,  20.0f));
            drawCloud(sceneShader, glm::vec3( 40.0f, 52.0f,  40.0f));
        } else {
            sceneShader.setInt("material.ignoreLighting", 2); // Celestial: Ignore all point lights
            // 1. The Moon (Existing Night Logic)
            glm::vec3 moonPos(40.0f, 65.0f, -95.0f);
            drawMesh(sphVAO, sphVerts, sceneShader.ID, glm::scale(glm::translate(glm::mat4(1.0f), moonPos), glm::vec3(7.0f)), glm::vec3(1.0f, 1.0f, 0.85f), 1.0f, 0, 1.0f, true, glm::vec3(0.7f, 0.7f, 0.5f)); 
            drawMesh(sphVAO, sphVerts, sceneShader.ID, glm::scale(glm::translate(glm::mat4(1.0f), moonPos + glm::vec3(2.0f, 3.0f, 5.0f)), glm::vec3(1.2f)), glm::vec3(0.6f, 0.55f, 0.3f), 1.0f, 0, 1.0f, true, glm::vec3(0.0f));
            drawMesh(sphVAO, sphVerts, sceneShader.ID, glm::scale(glm::translate(glm::mat4(1.0f), moonPos + glm::vec3(-3.0f, -1.5f, 5.5f)), glm::vec3(1.8f)), glm::vec3(0.6f, 0.55f, 0.3f), 1.0f, 0, 1.0f, true, glm::vec3(0.0f));
            drawMesh(sphVAO, sphVerts, sceneShader.ID, glm::scale(glm::translate(glm::mat4(1.0f), moonPos + glm::vec3(1.0f, -4.0f, 4.5f)), glm::vec3(0.9f)), glm::vec3(0.6f, 0.55f, 0.3f), 1.0f, 0, 1.0f, true, glm::vec3(0.0f));
            glDepthMask(GL_FALSE);
            drawMesh(sphVAO, sphVerts, sceneShader.ID, glm::scale(glm::translate(glm::mat4(1.0f), moonPos), glm::vec3(12.0f)), glm::vec3(1.0f, 0.95f, 0.6f), 1.0f, 0, 0.12f, true, glm::vec3(0.2f, 0.18f, 0.1f));
            glDepthMask(GL_TRUE);
            
            // 2. Stars
            for(int s=0; s < 120; ++s) {
                float seed = (float)s;
                float sx = sin(seed * 121.121f) * 220.0f;
                float sy = abs(cos(seed * 432.432f)) * 120.0f + 25.0f;
                float sz = sin(seed * 876.876f) * 220.0f;
                float pSize = 0.2f + (float)fmod(seed, 0.3f);
                float twinkle = 0.75f + 0.25f * sin(glfwGetTime() * 1.5f + seed);
                glm::vec3 starCol(1.0f);
                if(s % 10 == 0) starCol = glm::vec3(0.85f, 0.95f, 1.0f);
                drawMesh(cubeVAO, 36, sceneShader.ID, glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(sx, sy, sz)), glm::vec3(pSize)), starCol * twinkle, 1.0f, 0, 1.0f, false, starCol * twinkle);
            }
        }
        sceneShader.setInt("material.ignoreLighting", 0); // Reset for interior

        // 3. Ambient Dust Motes (Floating interior particles)
        // Tiny, low-alpha particles that drift slowly to add "air volume" realism
        for(int d=0; d < 80; ++d) {
            float dSeed = (float)d;
            float dx = sin(dSeed * 0.5f + glfwGetTime() * 0.1f) * 35.0f;
            float dy = 2.0f + abs(cos(dSeed * 0.8f)) * 15.0f;
            float dz = cos(dSeed * 0.3f + glfwGetTime() * 0.05f) * 35.0f;
            
            glDepthMask(GL_FALSE);
            drawMesh(cubeVAO, 36, sceneShader.ID, glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(dx, dy, dz)), glm::vec3(0.08f)), glm::vec3(1.0f), 0.1f, 0, 0.15f);
            glDepthMask(GL_TRUE);
        }
        // ------------------------        
        // 2 Trees flanking the building edges
        sceneShader.setInt("material.ignoreLighting", 1);
        drawTree(sceneShader, glm::vec3(-38.0f, 0.0f, 50.0f));
        drawTree(sceneShader, glm::vec3( 38.0f, 0.0f, 50.0f));
        sceneShader.setInt("material.ignoreLighting", 0);

        // ======================= WAREHOUSE ARCHITECTURE =======================
        glm::mat4 m(1.0f);
        // Huge Indoor Floor
        drawMesh(cubeVAO, 36, sceneShader.ID, glm::scale(glm::translate(m, glm::vec3(0,-0.5f,0)), glm::vec3(80.0f, 1.0f, 80.0f)), glm::vec3(1), 0.5f, useTexture ? floorTex : 0);
        // Roof Frame / Ceiling Slab (With Skylights)
        drawMesh(cubeVAO, 36, sceneShader.ID, glm::scale(glm::translate(m, glm::vec3(-25.0f,20.0f,0)), glm::vec3(30.0f, 2.0f, 80.0f)), glm::vec3(0.4f, 0.4f, 0.4f)); 
        drawMesh(cubeVAO, 36, sceneShader.ID, glm::scale(glm::translate(m, glm::vec3( 25.0f,20.0f,0)), glm::vec3(30.0f, 2.0f, 80.0f)), glm::vec3(0.4f, 0.4f, 0.4f)); 
        drawMesh(cubeVAO, 36, sceneShader.ID, glm::scale(glm::translate(m, glm::vec3( 0.0f,20.0f, 25.0f)), glm::vec3(20.0f, 2.0f, 30.0f)), glm::vec3(0.4f, 0.4f, 0.4f)); 
        drawMesh(cubeVAO, 36, sceneShader.ID, glm::scale(glm::translate(m, glm::vec3( 0.0f,20.0f,-25.0f)), glm::vec3(20.0f, 2.0f, 30.0f)), glm::vec3(0.4f, 0.4f, 0.4f)); 
        
        // Skylight Glass in the middle
        glDepthMask(GL_FALSE);
        drawMesh(cubeVAO, 36, sceneShader.ID, glm::scale(glm::translate(m, glm::vec3(0.0f, 20.0f, 0.0f)), glm::vec3(20.0f, 0.5f, 20.0f)), glm::vec3(0.4f, 0.6f, 0.8f), 1.0f, 0, 0.25f);
        glDepthMask(GL_TRUE);

        // Ceiling Wall Pipes
        for(float pZ = -38.0f; pZ <= 38.0f; pZ += 10.0f) { // Vertical holders
            drawMesh(cubeVAO, 36, sceneShader.ID, glm::scale(glm::translate(m, glm::vec3(-39.0f, 18.0f, pZ)), glm::vec3(2.0f, 0.2f, 0.2f)), glm::vec3(0.3f));
            drawMesh(cubeVAO, 36, sceneShader.ID, glm::scale(glm::translate(m, glm::vec3( 39.0f, 18.0f, pZ)), glm::vec3(2.0f, 0.2f, 0.2f)), glm::vec3(0.3f));
        }
        drawMesh(cylVAO, cylVerts, sceneShader.ID, glm::scale(glm::rotate(glm::translate(m, glm::vec3(-38.0f, 18.0f, 0.0f)), glm::radians(90.0f), glm::vec3(1,0,0)), glm::vec3(0.4f, 80.0f, 0.4f)), glm::vec3(0.7f, 0.7f, 0.8f)); // Left main pipe
        drawMesh(cylVAO, cylVerts, sceneShader.ID, glm::scale(glm::rotate(glm::translate(m, glm::vec3(-38.0f, 17.0f, 0.0f)), glm::radians(90.0f), glm::vec3(1,0,0)), glm::vec3(0.2f, 80.0f, 0.2f)), glm::vec3(0.9f, 0.3f, 0.1f)); // Left hot pipe
        drawMesh(cylVAO, cylVerts, sceneShader.ID, glm::scale(glm::rotate(glm::translate(m, glm::vec3( 38.0f, 18.0f, 0.0f)), glm::radians(90.0f), glm::vec3(1,0,0)), glm::vec3(0.4f, 80.0f, 0.4f)), glm::vec3(0.7f, 0.7f, 0.8f)); // Right main pipe
        
        // Massive Walls (Exterior & Interior)
        glm::vec3 offWhite(0.9f, 0.9f, 0.85f);
        // Exterior Layers
        sceneShader.setInt("material.ignoreLighting", 1);
        drawMesh(cubeVAO, 36, sceneShader.ID, glm::scale(glm::translate(m, glm::vec3(-40.0f, 10.0f, 0.0f)), glm::vec3(1.0f, 20.0f, 80.0f)), offWhite, 0.5f, useTexture ? wallTexOut : 0, 1.0f, false, glm::vec3(0.0f));
        drawMesh(cubeVAO, 36, sceneShader.ID, glm::scale(glm::translate(m, glm::vec3( 40.0f, 10.0f, 0.0f)), glm::vec3(1.0f, 20.0f, 80.0f)), offWhite, 0.5f, useTexture ? wallTexOut : 0, 1.0f, false, glm::vec3(0.0f));
        drawMesh(cubeVAO, 36, sceneShader.ID, glm::scale(glm::translate(m, glm::vec3(0.0f, 10.0f, -40.0f)), glm::vec3(80.0f, 20.0f, 1.0f)), offWhite, 0.5f, useTexture ? wallTexOut : 0, 1.0f, false, glm::vec3(0.0f));
        sceneShader.setInt("material.ignoreLighting", 0);
        // Interior Layers (Offset slightly inwards)
        drawMesh(cubeVAO, 36, sceneShader.ID, glm::scale(glm::translate(m, glm::vec3(-39.9f, 10.0f, 0.0f)), glm::vec3(0.2f, 20.0f, 79.8f)), offWhite, 0.5f, useTexture ? wallTexIn : 0);
        drawMesh(cubeVAO, 36, sceneShader.ID, glm::scale(glm::translate(m, glm::vec3( 39.9f, 10.0f, 0.0f)), glm::vec3(0.2f, 20.0f, 79.8f)), offWhite, 0.5f, useTexture ? wallTexIn : 0);
        drawMesh(cubeVAO, 36, sceneShader.ID, glm::scale(glm::translate(m, glm::vec3(0.0f, 10.0f, -39.9f)), glm::vec3(79.8f, 20.0f, 0.2f)), offWhite, 0.5f, useTexture ? wallTexIn : 0);
        
        // Front Portal featuring Sliding Door & Windows
        sceneShader.setInt("material.ignoreLighting", 1);
        drawMesh(cubeVAO, 36, sceneShader.ID, glm::scale(glm::translate(m, glm::vec3(-25.5f, 3.0f, 40.0f)), glm::vec3(29.0f, 6.0f, 1.0f)), offWhite, 0.5f, useTexture ? wallTexOut : 0, 1.0f, false, glm::vec3(0.0f)); // L Wall Bot
        drawMesh(cubeVAO, 36, sceneShader.ID, glm::scale(glm::translate(m, glm::vec3(-25.5f, 17.0f, 40.0f)), glm::vec3(29.0f, 6.0f, 1.0f)), offWhite, 0.5f, useTexture ? wallTexOut : 0, 1.0f, false, glm::vec3(0.0f)); // L Wall Top
        drawMesh(cubeVAO, 36, sceneShader.ID, glm::scale(glm::translate(m, glm::vec3(-36.5f, 10.0f, 40.0f)), glm::vec3(7.0f, 8.0f, 1.0f)), offWhite, 0.5f, useTexture ? wallTexOut : 0, 1.0f, false, glm::vec3(0.0f)); // L Wall Col 1
        drawMesh(cubeVAO, 36, sceneShader.ID, glm::scale(glm::translate(m, glm::vec3(-17.0f, 10.0f, 40.0f)), glm::vec3(12.0f, 8.0f, 1.0f)), offWhite, 0.5f, useTexture ? wallTexOut : 0, 1.0f, false, glm::vec3(0.0f)); // L Wall Col 2
        
        drawMesh(cubeVAO, 36, sceneShader.ID, glm::scale(glm::translate(m, glm::vec3( 25.5f, 3.0f, 40.0f)), glm::vec3(29.0f, 6.0f, 1.0f)), offWhite, 0.5f, useTexture ? wallTexOut : 0, 1.0f, false, glm::vec3(0.0f)); // R Wall Bot
        drawMesh(cubeVAO, 36, sceneShader.ID, glm::scale(glm::translate(m, glm::vec3( 25.5f, 17.0f, 40.0f)), glm::vec3(29.0f, 6.0f, 1.0f)), offWhite, 0.5f, useTexture ? wallTexOut : 0, 1.0f, false, glm::vec3(0.0f)); // R Wall Top
        drawMesh(cubeVAO, 36, sceneShader.ID, glm::scale(glm::translate(m, glm::vec3( 17.0f, 10.0f, 40.0f)), glm::vec3(12.0f, 8.0f, 1.0f)), offWhite, 0.5f, useTexture ? wallTexOut : 0, 1.0f, false, glm::vec3(0.0f)); // R Wall Col 1
        drawMesh(cubeVAO, 36, sceneShader.ID, glm::scale(glm::translate(m, glm::vec3( 36.5f, 10.0f, 40.0f)), glm::vec3(7.0f, 8.0f, 1.0f)), offWhite, 0.5f, useTexture ? wallTexOut : 0, 1.0f, false, glm::vec3(0.0f)); // R Wall Col 2

        drawMesh(cubeVAO, 36, sceneShader.ID, glm::scale(glm::translate(m, glm::vec3(  0.0f, 17.5f, 40.0f)), glm::vec3(22.0f,  5.0f, 1.0f)), offWhite, 0.5f, useTexture ? wallTexOut : 0, 1.0f, false, glm::vec3(0.0f)); // Top Gate Frame
        
        // Two glowing bulbs flanking the entrance
        drawMesh(sphVAO, sphVerts, sceneShader.ID, glm::scale(glm::translate(m, glm::vec3(-12.5f, 15.0f, 40.5f)), glm::vec3(0.4f)), glm::vec3(1.0f, 1.0f, 0.8f), 1.0f, 0, 1.0f, true, glm::vec3(1.0f, 1.0f, 0.5f));
        drawMesh(sphVAO, sphVerts, sceneShader.ID, glm::scale(glm::translate(m, glm::vec3( 12.5f, 15.0f, 40.5f)), glm::vec3(0.4f)), glm::vec3(1.0f, 1.0f, 0.8f), 1.0f, 0, 1.0f, true, glm::vec3(1.0f, 1.0f, 0.5f));
        sceneShader.setInt("material.ignoreLighting", 0);
        
        // Poster and Hazard Marks removed.
        
        // Nameplate "3D WAREHOUSE" Map
        sceneShader.setInt("material.ignoreLighting", 1);
        drawMesh(cubeVAO, 36, sceneShader.ID, glm::scale(glm::translate(m, glm::vec3(0.0f, 17.5f, 40.8f)), glm::vec3(14.0f, -3.5f, 0.2f)), glm::vec3(1.0f), 0.5f, useTexture ? nameTex : 0);
        sceneShader.setInt("material.ignoreLighting", 0);

        // SLIDING GARAGE DOOR (Animated horizontally by 'O' and 'P')
        sceneShader.setInt("material.ignoreLighting", 1);
        drawMesh(cubeVAO, 36, sceneShader.ID, glm::scale(glm::translate(m, glm::vec3(0.0f - doorX, 7.5f, 40.0f)), glm::vec3(22.0f, 15.0f, 0.6f)), glm::vec3(0.5f, 0.6f, 0.7f), 0.5f, useTexture ? doorTex : 0, 1.0f, false, glm::vec3(0.0f)); 
        sceneShader.setInt("material.ignoreLighting", 0);
        
        // Front Glass Windows (Moved to interior mounting at Z=39.4f)
        // Frames
        drawMesh(cubeVAO, 36, sceneShader.ID, glm::scale(glm::translate(m, glm::vec3(-28.0f, 10.0f, 39.45f)), glm::vec3(1.0f, 8.0f, 0.6f)), glm::vec3(0.2f)); // Mid vertical frame left
        drawMesh(cubeVAO, 36, sceneShader.ID, glm::scale(glm::translate(m, glm::vec3(-28.0f, 10.0f, 39.45f)), glm::vec3(10.0f, 1.0f, 0.6f)), glm::vec3(0.2f)); // Mid horizontal frame left
        drawMesh(cubeVAO, 36, sceneShader.ID, glm::scale(glm::translate(m, glm::vec3( 28.0f, 10.0f, 39.45f)), glm::vec3(1.0f, 8.0f, 0.6f)), glm::vec3(0.2f)); // Mid vertical frame right
        drawMesh(cubeVAO, 36, sceneShader.ID, glm::scale(glm::translate(m, glm::vec3( 28.0f, 10.0f, 39.45f)), glm::vec3(10.0f, 1.0f, 0.6f)), glm::vec3(0.2f)); // Mid horizontal frame right
        
        // Glass
        glDepthMask(GL_FALSE);
        drawMesh(cubeVAO, 36, sceneShader.ID, glm::scale(glm::translate(m, glm::vec3(-28.0f, 10.0f + windowY, 39.4f)), glm::vec3(10.0f, 8.0f, 0.8f)), glm::vec3(0.3f, 0.7f, 0.9f), 1.0f, 0, 0.35f); 
        drawMesh(cubeVAO, 36, sceneShader.ID, glm::scale(glm::translate(m, glm::vec3( 28.0f, 10.0f + windowY, 39.4f)), glm::vec3(10.0f, 8.0f, 0.8f)), glm::vec3(0.3f, 0.7f, 0.9f), 1.0f, 0, 0.35f); 
        glDepthMask(GL_TRUE);

        // ======================= POPULATING EMPTY SPACE =======================
        // Vending Machine (Near left window)
        glm::mat4 vM = glm::translate(glm::mat4(1.0f), glm::vec3(-22.0f, 3.0f, 35.0f));
        drawMesh(cubeVAO, 36, sceneShader.ID, glm::scale(vM, glm::vec3(5.0f, 6.0f, 3.0f)), glm::vec3(0.8f, 0.1f, 0.1f)); // Red Body
        drawMesh(cubeVAO, 36, sceneShader.ID, glm::scale(glm::translate(vM, glm::vec3(1.5f, 0.0f, 1.55f)), glm::vec3(1.0f, 6.0f, 0.1f)), glm::vec3(0.2f)); // Control strip
        glDepthMask(GL_FALSE);
        drawMesh(cubeVAO, 36, sceneShader.ID, glm::scale(glm::translate(vM, glm::vec3(-0.8f, 0.5f, 1.6f)), glm::vec3(3.4f, 4.5f, 0.2f)), glm::vec3(0.7f, 0.9f, 1.0f), 1.0f, 0, 0.4f); // Glass
        glDepthMask(GL_TRUE);

        // Control Center (Near right window)
        glm::mat4 dM = glm::translate(glm::mat4(1.0f), glm::vec3(24.0f, 1.5f, 32.0f));
        // Desk Body
        drawMesh(cubeVAO, 36, sceneShader.ID, glm::scale(dM, glm::vec3(9.0f, 0.4f, 4.5f)), glm::vec3(0.5f, 0.35f, 0.2f)); // Wood Desk surface
        drawMesh(cubeVAO, 36, sceneShader.ID, glm::scale(glm::translate(dM, glm::vec3( 3.5f, -0.6f, 0.0f)), glm::vec3(2.0f, 1.0f, 4.5f)), glm::vec3(0.3f)); // Right Drawers
        drawMesh(cylVAO, cylVerts, sceneShader.ID, glm::scale(glm::translate(dM, glm::vec3(-4.0f,-0.75f, 1.8f)), glm::vec3(0.15f, 1.5f, 0.15f)), glm::vec3(0.2f)); // Metal Leg
        drawMesh(cylVAO, cylVerts, sceneShader.ID, glm::scale(glm::translate(dM, glm::vec3(-4.0f,-0.75f,-1.8f)), glm::vec3(0.15f, 1.5f, 0.15f)), glm::vec3(0.2f)); // Metal Leg
        drawMesh(cylVAO, cylVerts, sceneShader.ID, glm::scale(glm::translate(dM, glm::vec3( 3.5f,-1.3f, 1.8f)), glm::vec3(0.15f, 0.4f, 0.15f)), glm::vec3(0.2f)); // Right Leg
        drawMesh(cylVAO, cylVerts, sceneShader.ID, glm::scale(glm::translate(dM, glm::vec3( 3.5f,-1.3f,-1.8f)), glm::vec3(0.15f, 0.4f, 0.15f)), glm::vec3(0.2f)); // Right Leg
        
        // PC & Peripherals
        drawMesh(cubeVAO, 36, sceneShader.ID, glm::scale(glm::translate(dM, glm::vec3(-3.0f, 1.2f, -0.5f)), glm::vec3(1.5f, 2.0f, 3.5f)), glm::vec3(0.1f)); // PC Tower
        drawMesh(cubeVAO, 36, sceneShader.ID, glm::scale(glm::translate(dM, glm::vec3(0.5f,  0.25f, 1.0f)), glm::vec3(2.5f, 0.1f, 0.8f)), glm::vec3(0.2f)); // Keyboard base
        drawMesh(cubeVAO, 36, sceneShader.ID, glm::scale(glm::translate(dM, glm::vec3(0.5f,  0.31f, 1.0f)), glm::vec3(2.3f, 0.05f, 0.6f)), glm::vec3(0.8f, 0.2f, 0.9f)); // Glowing Keyboard keys
        drawMesh(cubeVAO, 36, sceneShader.ID, glm::scale(glm::translate(dM, glm::vec3(2.5f,  0.25f, 1.0f)), glm::vec3(0.4f, 0.1f, 0.6f)), glm::vec3(0.15f)); // Mouse

        // Monitors with exact bases
        glm::mat4 mL = glm::rotate(glm::translate(dM, glm::vec3(-1.0f, 1.2f, -0.8f)), glm::radians(15.0f), glm::vec3(0,1,0));
        drawMesh(cylVAO, cylVerts, sceneShader.ID, glm::scale(glm::translate(mL, glm::vec3(0, -0.6f, 0)), glm::vec3(0.1f, 0.8f, 0.1f)), glm::vec3(0.1f)); // Base Stem
        drawMesh(cubeVAO, 36, sceneShader.ID, glm::scale(mL, glm::vec3(3.0f, 1.6f, 0.2f)), glm::vec3(0.05f)); // Frame
        drawMesh(cubeVAO, 36, sceneShader.ID, glm::scale(glm::translate(mL, glm::vec3(0,0,0.11f)), glm::vec3(2.8f, 1.4f, 0.05f)), glm::vec3(0.2f, 0.6f, 0.9f)); // L Screen Output

        glm::mat4 mR = glm::rotate(glm::translate(dM, glm::vec3( 2.2f, 1.2f, -0.8f)), glm::radians(-15.0f), glm::vec3(0,1,0));
        drawMesh(cylVAO, cylVerts, sceneShader.ID, glm::scale(glm::translate(mR, glm::vec3(0, -0.6f, 0)), glm::vec3(0.1f, 0.8f, 0.1f)), glm::vec3(0.1f)); // Base Stem
        drawMesh(cubeVAO, 36, sceneShader.ID, glm::scale(mR, glm::vec3(3.0f, 1.6f, 0.2f)), glm::vec3(0.05f)); // Frame
        drawMesh(cubeVAO, 36, sceneShader.ID, glm::scale(glm::translate(mR, glm::vec3(0,0,0.11f)), glm::vec3(2.8f, 1.4f, 0.05f)), glm::vec3(0.2f, 0.6f, 0.9f)); // R Screen Output

        // Office Chair (Swivel)
        glm::mat4 cM = glm::translate(dM, glm::vec3(0.0f, -0.5f, 3.5f));
        drawMesh(cylVAO, cylVerts, sceneShader.ID, glm::scale(cM, glm::vec3(0.2f, 1.0f, 0.2f)), glm::vec3(0.2f)); // Central Stem
        drawMesh(cubeVAO, 36, sceneShader.ID, glm::scale(glm::translate(cM, glm::vec3(0,-0.5f,0)), glm::vec3(1.8f, 0.2f, 0.4f)), glm::vec3(0.1f)); // Wheel spoke 1
        drawMesh(cubeVAO, 36, sceneShader.ID, glm::scale(glm::translate(cM, glm::vec3(0,-0.5f,0)), glm::vec3(0.4f, 0.2f, 1.8f)), glm::vec3(0.1f)); // Wheel spoke 2
        drawMesh(cubeVAO, 36, sceneShader.ID, glm::scale(glm::translate(cM, glm::vec3(0.0f, 0.5f, 0.0f)), glm::vec3(2.5f, 0.4f, 2.5f)), glm::vec3(0.15f, 0.15f, 0.15f)); // Cushioned Seat
        drawMesh(cubeVAO, 36, sceneShader.ID, glm::scale(glm::translate(cM, glm::vec3(0.0f, 1.8f, 1.1f)), glm::vec3(2.2f, 2.5f, 0.4f)), glm::vec3(0.15f, 0.15f, 0.15f)); // Backrest
        drawMesh(cubeVAO, 36, sceneShader.ID, glm::scale(glm::translate(cM, glm::vec3(-1.1f, 1.2f, 0.0f)), glm::vec3(0.2f, 0.2f, 2.0f)), glm::vec3(0.05f)); // L Armrest
        drawMesh(cubeVAO, 36, sceneShader.ID, glm::scale(glm::translate(cM, glm::vec3( 1.1f, 1.2f, 0.0f)), glm::vec3(0.2f, 0.2f, 2.0f)), glm::vec3(0.05f)); // R Armrest

        // ======================= INTERIOR FIXTURES (Lights & Fans) =======================
        for (int i = 0; i < NR_POINT_LIGHTS; i++) {
            // Draw visual representation of ceiling lights (Fluorescent housings)
            drawMesh(cubeVAO, 36, sceneShader.ID, glm::scale(glm::translate(glm::mat4(1.0f), pointLightPositions[i] + glm::vec3(0,0.5f,0)), glm::vec3(4.0f, 0.2f, 1.0f)), glm::vec3(1.0f, 1.0f, 1.0f)); 
        }

        // Multiple Ceiling Fans Hanging from roof
        for(float fz = -20; fz <= 20; fz += 20) {
            glm::mat4 fBase = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 18.0f, fz));
            drawMesh(cylVAO, cylVerts, sceneShader.ID, glm::scale(fBase, glm::vec3(0.4f, 4.0f, 0.4f)), glm::vec3(0.1f)); // Stem
            glm::mat4 fRotor = glm::rotate(glm::translate(fBase, glm::vec3(0,-2.0f,0)), glm::radians(fanAngle), glm::vec3(0,1,0));
            drawMesh(cubeVAO, 36, sceneShader.ID, glm::scale(glm::translate(fRotor, glm::vec3(4,0,0)), glm::vec3(8.0f, 0.1f, 1.5f)), glm::vec3(0.8f));
            drawMesh(cubeVAO, 36, sceneShader.ID, glm::scale(glm::translate(fRotor, glm::vec3(-4,0,0)), glm::vec3(8.0f, 0.1f, 1.5f)), glm::vec3(0.8f));
            drawMesh(cubeVAO, 36, sceneShader.ID, glm::scale(glm::translate(fRotor, glm::vec3(0,0,4)), glm::vec3(1.5f, 0.1f, 8.0f)), glm::vec3(0.8f));
            drawMesh(cubeVAO, 36, sceneShader.ID, glm::scale(glm::translate(fRotor, glm::vec3(0,0,-4)), glm::vec3(1.5f, 0.1f, 8.0f)), glm::vec3(0.8f));
        }

        // ======================= RACKS & BOXES =======================
        auto drawIndustrialRack = [&](glm::vec3 rp) {
            glm::mat4 bm = glm::translate(glm::mat4(1.0f), rp);
            // Draw horizontal shelves
            for(int k=0; k<=18; k+=6) drawMesh(cubeVAO, 36, sceneShader.ID, glm::scale(glm::translate(bm, glm::vec3(0,k,0)), glm::vec3(26, 0.5f, 6)), glm::vec3(0.2f, 0.3f, 0.6f));
            // Vertical Posts
            for(int x=-13; x<=13; x+=13) for(int z=-3; z<=3; z+=6) drawMesh(cubeVAO, 36, sceneShader.ID, glm::scale(glm::translate(bm, glm::vec3(x, 9, z)), glm::vec3(0.6f, 18.0f, 0.6f)), glm::vec3(0.2f, 0.3f, 0.6f));
            // Stacked Cardboard Packages on shelves
            for(int k=1; k<=13; k+=6) {
                for(int x=-10; x<=10; x+=5) {
                    float bx1 = 3.5f, by1 = 3.0f, bz1 = 3.5f;
                    float bx2 = 2.5f, by2 = 2.0f, bz2 = 2.5f;
                    if((x+k)%3==0) { bx1 = 2.0f; by1 = 4.0f; bz1 = 2.0f; }
                    if((x+k)%5==0) { bx2 = 4.0f; by2 = 1.5f; bz2 = 4.0f; }
                    
                    drawMesh(cubeVAO, 36, sceneShader.ID, glm::scale(glm::translate(bm, glm::vec3(x, k + by1/2.0f,  1)), glm::vec3(bx1, by1, bz1)), glm::vec3(0.7f, 0.6f, 0.4f)); 
                    drawMesh(cubeVAO, 36, sceneShader.ID, glm::scale(glm::translate(bm, glm::vec3(x+1, k + by2/2.0f, -1)), glm::vec3(bx2, by2, bz2)), glm::vec3(0.8f, 0.7f, 0.5f)); 
                }
            }
        };
        // Place shelving deeply but cleanly inside the warehouse spatial X-boundaries
        drawIndustrialRack(glm::vec3(-24.0f, 0.0f,  0.0f));
        drawIndustrialRack(glm::vec3(-24.0f, 0.0f, -25.0f));
        drawIndustrialRack(glm::vec3( 24.0f, 0.0f, -25.0f));

        // ======================= THE CONTROLLABLE FORKLIFT =======================
        // Use translation & rotation generated from Arrow Keys
        glm::mat4 car = glm::rotate(glm::translate(glm::mat4(1.0f), carPos), glm::radians(carRotation), glm::vec3(0,1,0));
        
        drawMesh(cubeVAO, 36, sceneShader.ID, glm::scale(glm::translate(car, glm::vec3(0,2.0f,0)), glm::vec3(6.0f, 3.0f, 10.0f)), glm::vec3(0.9f, 0.7f, 0.1f)); // Main Body (Yellow)
        drawMesh(cubeVAO, 36, sceneShader.ID, glm::scale(glm::translate(car, glm::vec3(0,6.0f,-1.0f)), glm::vec3(5.5f, 5.0f, 4.0f)), glm::vec3(0.2f, 0.2f, 0.2f)); // Cabin
        drawMesh(cubeVAO, 36, sceneShader.ID, glm::scale(glm::translate(car, glm::vec3(0,5.0f,5.0f)), glm::vec3(4.0f, 10.0f, 0.8f)), glm::vec3(0.1f, 0.1f, 0.1f)); // Lift Mast
        drawMesh(cubeVAO, 36, sceneShader.ID, glm::scale(glm::translate(car, glm::vec3(1.5f, 0.5f + forkY, 8.0f)), glm::vec3(0.5f, 0.2f, 5.0f)), glm::vec3(0.7f)); // Right Fork
        drawMesh(cubeVAO, 36, sceneShader.ID, glm::scale(glm::translate(car, glm::vec3(-1.5f, 0.5f + forkY, 8.0f)), glm::vec3(0.5f, 0.2f, 5.0f)), glm::vec3(0.7f)); // Left Fork
        // Forklift Wheels (With metallic rim)
        for(float x : {-3.0f, 3.0f}) 
            for(float z : {-3.5f, 3.5f}) {
                glm::mat4 wMat = glm::translate(car, glm::vec3(x, 1.0f, z));
                wMat = glm::rotate(wMat, glm::radians(carWheelRot), glm::vec3(1,0,0)); // Rolling
                wMat = glm::rotate(wMat, glm::radians(90.0f), glm::vec3(0,0,1)); // Cylinder orientation
                drawMesh(cylVAO, cylVerts, sceneShader.ID, glm::scale(wMat, glm::vec3(2.0f, 1.0f, 2.0f)), glm::vec3(0.05f));
                drawMesh(cylVAO, cylVerts, sceneShader.ID, glm::scale(wMat, glm::vec3(1.2f, 1.05f, 1.2f)), glm::vec3(0.6f, 0.6f, 0.65f)); // Silver Rim
            }
        // Payload currently on forklift
        drawMesh(cubeVAO, 36, sceneShader.ID, glm::scale(glm::translate(car, glm::vec3(0.0f, 2.5f + forkY, 8.0f)), glm::vec3(3.5f, 3.5f, 3.5f)), glm::vec3(0.8f, 0.6f, 0.4f)); 

        // ======================= REALISTIC ROBOTIC SORTING ARM =======================
        // State Machine: 10 second loop over animTime (0 to 10)
        float baseRot = 0.0f, shoulderRot = 45.0f, elbowRot = -30.0f, pincerDist = 1.5f;
        bool hasBox = false;
        
        if (!animationRunning) animTime = 0.0f;
        if (animTime < 2.0f) { // Fetch (Rotate down to conveyor)
            baseRot = 0.0f; shoulderRot = 60.0f; elbowRot = -50.0f; pincerDist = 2.5f;
        } else if (animTime < 3.0f) { // Grasp
            baseRot = 0.0f; shoulderRot = 60.0f; elbowRot = -50.0f; pincerDist = 0.5f; hasBox = true;
        } else if (animTime < 6.0f) { // Transfer (Lift and rotate 180 deg)
            float t = (animTime - 3.0f) / 3.0f; // 0 to 1
            baseRot = t * 180.0f; shoulderRot = 60.0f - t * 45.0f; elbowRot = -50.0f + t * 40.0f; pincerDist = 0.5f; hasBox = true;
        } else if (animTime < 8.0f) { // Lower to drop
            baseRot = 180.0f; shoulderRot = 50.0f; elbowRot = -40.0f; pincerDist = 0.5f; hasBox = true;
        } else if (animTime < 9.0f) { // Release
            baseRot = 180.0f; shoulderRot = 50.0f; elbowRot = -40.0f; pincerDist = 2.5f; hasBox = false;
        } else { // Reset
            float t = (animTime - 9.0f) / 1.0f; // 0 to 1
            baseRot = 180.0f + t * 180.0f; shoulderRot = 50.0f * (1.0f - t); elbowRot = -40.0f * (1.0f - t); pincerDist = 2.5f;
        }

        glm::mat4 rBase = glm::rotate(glm::translate(glm::mat4(1.0f), glm::vec3(10.0f, 0.75f, -15.0f)), glm::radians(baseRot), glm::vec3(0,1,0));
        drawMesh(cylVAO, cylVerts, sceneShader.ID, glm::scale(rBase, glm::vec3(2.5f, 1.5f, 2.5f)), glm::vec3(0.2f, 0.3f, 0.4f)); 
        
        glm::mat4 armBicep = glm::rotate(glm::translate(rBase, glm::vec3(0,1.0f,0)), glm::radians(shoulderRot), glm::vec3(1,0,0));
        drawMesh(cubeVAO, 36, sceneShader.ID, glm::scale(glm::translate(armBicep, glm::vec3(0,3.0f,0)), glm::vec3(0.8f, 6.0f, 0.8f)), glm::vec3(0.9f, 0.5f, 0.1f)); 
        
        glm::mat4 armFore = glm::rotate(glm::translate(armBicep, glm::vec3(0,6.0f,0)), glm::radians(elbowRot), glm::vec3(1,0,0));
        drawMesh(cylVAO, cylVerts, sceneShader.ID, glm::scale(glm::rotate(armFore, glm::radians(90.0f), glm::vec3(0,0,1)), glm::vec3(1.2f, 1.0f, 1.2f)), glm::vec3(0.2f)); 
        drawMesh(cubeVAO, 36, sceneShader.ID, glm::scale(glm::translate(armFore, glm::vec3(0,2.5f,0)), glm::vec3(0.6f, 5.0f, 0.6f)), glm::vec3(0.9f, 0.5f, 0.1f)); 
        
        glm::mat4 claw = glm::translate(armFore, glm::vec3(0,5.0f,0));
        drawMesh(cubeVAO, 36, sceneShader.ID, glm::scale(claw, glm::vec3(1.8f, 0.3f, 1.8f)), glm::vec3(0.3f)); // Hand plate
        drawMesh(cubeVAO, 36, sceneShader.ID, glm::scale(glm::translate(claw, glm::vec3( pincerDist*0.7f, 1.0f, 0)), glm::vec3(0.15f, 2.0f, 0.15f)), glm::vec3(0.1f)); // Pincer 1
        drawMesh(cubeVAO, 36, sceneShader.ID, glm::scale(glm::translate(claw, glm::vec3(-pincerDist*0.7f, 1.0f, 0)), glm::vec3(0.15f, 2.0f, 0.15f)), glm::vec3(0.1f)); // Pincer 2
        
        if (hasBox) {
            drawMesh(cubeVAO, 36, sceneShader.ID, glm::scale(glm::translate(claw, glm::vec3(0, 1.0f, 0)), glm::vec3(1.8f, 1.8f, 1.8f)), glm::vec3(0.8f, 0.7f, 0.5f)); 
        }

        // --- DYNAMIC SPARKS (Emissive Particles) ---
        // Spawns bursting orange sparks when grabbing/releasing (States 1-3 and 8-9)
        if((animTime > 1.8f && animTime < 2.5f) || (animTime > 8.8f && animTime < 9.5f)) {
            for(int p=0; p<15; ++p) {
                float pSeed = (float)p + glfwGetTime();
                glm::vec3 pOff(sin(pSeed*10.0f)*1.5f, cos(pSeed*7.0f)*1.5f, sin(pSeed*5.0f)*1.5f);
                drawMesh(cubeVAO, 36, sceneShader.ID, glm::scale(glm::translate(claw, glm::vec3(0, 1.0f, 0) + pOff), glm::vec3(0.15f)), glm::vec3(1, 0.5f, 0), 1.0f, 0, 1.0f, false, glm::vec3(1, 0.4f, 0));
            }
        }
        
        // Drop zone pallet
        drawMesh(cubeVAO, 36, sceneShader.ID, glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(10.0f, 0.5f, 5.0f)), glm::vec3(6.0f, 1.0f, 6.0f)), glm::vec3(0.5f, 0.3f, 0.1f)); 

        // ======================= THE CONVEYOR BELT PLATFORM =======================
        glm::mat4 conv = glm::translate(glm::mat4(1.0f), glm::vec3(10.0f, 1.5f, -25.0f));
        drawMesh(cubeVAO, 36, sceneShader.ID, glm::scale(conv, glm::vec3(6.0f, 0.2f, 30.0f)), glm::vec3(0.2f)); // Frame left/right structure
        // Dozens of metallic rolling cylinders
        for(float cZ = -14.0f; cZ <= 14.0f; cZ += 1.5f) {
            glm::mat4 rMat = glm::translate(conv, glm::vec3(0, 0.2f, cZ));
            rMat = glm::rotate(rMat, glm::radians(fanAngle), glm::vec3(1,0,0)); // Simulate rolling via fanAngle
            rMat = glm::rotate(rMat, glm::radians(90.0f), glm::vec3(0,0,1));
            drawMesh(cylVAO, cylVerts, sceneShader.ID, glm::scale(rMat, glm::vec3(0.6f, 5.8f, 0.6f)), glm::vec3(0.6f, 0.6f, 0.65f));
        }
        
        // Loop multiple boxes across the conveyor continuously using animTime 
        // animTime relies on a 10s loop, moving 30 units total length
        for(int i = 0; i < 4; i++) {
            float boxTimeOffset = animTime + i * 2.5f; 
            float continuousZ = fmod(boxTimeOffset, 10.0f) * 3.0f - 15.0f; // -15 to +15 bounding relative to conveyor center
            drawMesh(cubeVAO, 36, sceneShader.ID, glm::scale(glm::translate(conv, glm::vec3(0.0f, 1.5f, continuousZ)), glm::vec3(3.0f, 2.5f, 3.0f)), glm::vec3(0.8f, 0.7f, 0.5f)); 
        }

        glfwSwapBuffers(window); 
        glfwPollEvents();
    }
    glfwTerminate(); return 0;
}
