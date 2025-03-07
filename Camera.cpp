#include "Camera.hpp"
#include <GLFW/glfw3.h>

namespace gps {
    bool animationActive = false;

    //Camera constructor
    Camera::Camera(glm::vec3 cameraPosition, glm::vec3 cameraTarget, glm::vec3 cameraUp) {
        //TODO
        this->cameraPosition = cameraPosition;
        this->cameraTarget = cameraTarget;
        this->cameraUpDirection = cameraUp;
        
    }

    //return the view matrix, using the glm::lookAt() function
    glm::mat4 Camera::getViewMatrix() {
        //TODO

        return glm::lookAt(cameraPosition, cameraTarget, this->cameraUpDirection);
    }

    //update the camera internal parameters following a camera move event
    void Camera::move(MOVE_DIRECTION direction, float speed) {
        glm::vec3 forward = glm::normalize(cameraTarget - cameraPosition);
        glm::vec3 right = glm::normalize(glm::cross(forward, cameraUpDirection));

        switch (direction) {
        case MOVE_FORWARD:
            cameraPosition += forward * speed;
            cameraTarget += forward * speed;
            break;
        case MOVE_BACKWARD:
            cameraPosition -= forward * speed;
            cameraTarget -= forward * speed;
            break;
        case MOVE_LEFT:
            cameraPosition -= right * speed;
            cameraTarget -= right * speed;
            break;
        case MOVE_RIGHT:
            cameraPosition += right * speed;
            cameraTarget += right * speed;
            break;
        }
    }


    //update the camera internal parameters following a camera rotate event
    //yaw - camera rotation around the y axis
    //pitch - camera rotation around the x axis
    void Camera::rotate(float pitch, float yaw) {
        glm::vec3 forward = glm::normalize(cameraTarget - cameraPosition);

        // Convert the forward vector into spherical coordinates
        float radius = glm::length(forward);
        float currentYaw = atan2(forward.z, forward.x);
        float currentPitch = asin(forward.y);

        // Update the yaw and pitch
        currentYaw += glm::radians(yaw);
        currentPitch += glm::radians(pitch);

        // Clamp the pitch to prevent flipping
        float maxPitch = glm::radians(89.0f); // Limit to 89 degrees
        currentPitch = glm::clamp(currentPitch, -maxPitch, maxPitch);

        // Convert spherical coordinates back to Cartesian coordinates
        forward.x = radius * cos(currentPitch) * cos(currentYaw);
        forward.y = radius * sin(currentPitch);
        forward.z = radius * cos(currentPitch) * sin(currentYaw);

        // Update the camera target
        cameraTarget = cameraPosition + forward;
    }


    void Camera::toggleAnimation() {
        animationActive = !animationActive;
    }

    bool Camera::isAnimationActive()
    {
        return animationActive;
    }


    //void Camera::animateCamera(float deltaTime, const std::vector<glm::vec3>& controlPoints) {
    //    static float t = 0.0f;  // Interpolation parameter (0.0 to 1.0)
    //    float speed = 0.05f;    // Speed of camera movement (adjust as needed)

    //    // Update `t` based on speed and deltaTime
    //    t += speed * deltaTime;
    //    if (t > 1.0f) t -= 1.0f; // Loop back to start

    //    // Calculate position on the Bezier curve using De Casteljau's algorithm
    //    glm::vec3 p0 = glm::mix(controlPoints[0], controlPoints[1], t);
    //    glm::vec3 p1 = glm::mix(controlPoints[1], controlPoints[2], t);
    //    glm::vec3 p2 = glm::mix(controlPoints[2], controlPoints[3], t);
    //    glm::vec3 p3 = glm::mix(controlPoints[3], controlPoints[0], t);

    //    glm::vec3 p4 = glm::mix(p0, p1, t);
    //    glm::vec3 p5 = glm::mix(p1, p2, t);
    //    glm::vec3 p6 = glm::mix(p2, p3, t);

    //    glm::vec3 p7 = glm::mix(p4, p5, t);
    //    glm::vec3 p8 = glm::mix(p5, p6, t);

    //    cameraPosition = glm::mix(p7, p8, t);

    //    // Always face the center of the scene
    //    cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
    //}
    void Camera::setPosition(float x, float y, float z) {
        // Update the camera position
        cameraPosition = glm::vec3(x, y, z);

        // Maintain the same camera direction by keeping the target fixed
        // Ensure the cameraTarget does not depend on the old position
        
    }



    void Camera::setDirection(const glm::vec3& target) {
        // Actualizăm cameraTarget pentru a reflecta noua direcție
        cameraTarget = glm::normalize(target);

        // Asigurăm că vectorul în sus rămâne definit
        cameraUpDirection = glm::vec3(0.0f, 1.0f, 0.0f);
    }




}
