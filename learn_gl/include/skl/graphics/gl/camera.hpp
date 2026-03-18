#pragma once

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace skl::opengl {
constexpr GLfloat YAW = -90.0F;
constexpr GLfloat PITCH = 0.0F;
constexpr GLfloat SPEED = 2.5F;
constexpr GLfloat SEN = 0.1F;
constexpr GLfloat ZOOM = 45.0F;

class Camera {
private:
    // camera Attributes
    glm::vec3 _position{};
    glm::vec3 _front{};
    glm::vec3 _up{};
    glm::vec3 _right{};
    glm::vec3 _worldUp{};
    // euler Angles
    GLfloat _yaw;
    GLfloat _pitch;
    // camera options
    GLfloat _moveSpeed;
    GLfloat _mouseSen;
    GLfloat _zoom;

    void updateCameraVectors() noexcept;

public:
    enum Movement : uint8_t { FRONT, BACK, LEFT, RIGHT };

    Camera(glm::vec3 position = glm::vec3(0.0F, 0.0F, 0.0F), glm::vec3 worldup = glm::vec3(0.0F, 1.0F, 0.0F),
           GLfloat yaw = YAW, GLfloat pitch = PITCH) noexcept;
    Camera(GLfloat posX, GLfloat posY, GLfloat posZ, GLfloat upX, GLfloat upY, GLfloat upZ, GLfloat yaw, GLfloat pitch) noexcept;
    ~Camera() noexcept = default;

    [[nodiscard]] const glm::vec3& getPosition() const noexcept;
    void setPosition(const glm::vec3& position) noexcept;

    [[nodiscard]] const glm::vec3& getFront() const noexcept;
    [[nodiscard]] const glm::vec3& getUp() const noexcept;
    [[nodiscard]] const glm::vec3& getRight() const noexcept;

    [[nodiscard]] GLfloat getYaw() const noexcept;
    void setYaw(GLfloat yaw) noexcept;

    [[nodiscard]] GLfloat getPitch() const noexcept;
    void setPitch(GLfloat pitch) noexcept;

    [[nodiscard]] GLfloat getMoveSpeed() const noexcept;
    void setMoveSpeed(GLfloat moveSpeed) noexcept;

    [[nodiscard]] GLfloat getMouseSen() const noexcept;
    void setMouseSen(GLfloat mouseSen) noexcept;

    [[nodiscard]] GLfloat getZoom() const noexcept;
    void setZoom(GLfloat zoom) noexcept;

    glm::mat4 getView() noexcept;
    void keyboard(Camera::Movement direction, GLfloat deltaTime) noexcept;
    void mouseMove(GLfloat xoffset, GLfloat yoffset, GLboolean constrainPitch = GL_TRUE) noexcept;
    void mouseScroll(GLfloat yoffset) noexcept;
    void update() noexcept;
};
}   // namespace skl::opengl
