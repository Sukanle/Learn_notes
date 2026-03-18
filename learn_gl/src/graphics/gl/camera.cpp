#include "skl/graphics/gl/camera.hpp"

namespace skl::opengl {
void Camera::updateCameraVectors() noexcept {
    glm::vec3 front;
    front.x = cosf(glm::radians(_yaw)) * cosf(glm::radians(_pitch));
    front.y = sinf(glm::radians(_pitch));
    front.z = sinf(glm::radians(_yaw)) * cosf(glm::radians(_pitch));
    _front = glm::normalize(front);
    _right = glm::normalize(glm::cross(_front, _worldUp));
    _up = glm::normalize(glm::cross(_right, _front));
}

Camera::Camera(glm::vec3 position, glm::vec3 worldup, GLfloat yaw, GLfloat pitch) noexcept
    : _position(position)
    , _front(glm::vec3(0.0F, 0.0F, -1.0F))
    , _worldUp(worldup)
    , _yaw(yaw)
    , _pitch(pitch)
    , _moveSpeed(SPEED)
    , _mouseSen(SEN)
    , _zoom(ZOOM) {
    updateCameraVectors();
}

Camera::Camera(GLfloat posX, GLfloat posY, GLfloat posZ, GLfloat upX, GLfloat upY, GLfloat upZ, GLfloat yaw, GLfloat pitch) noexcept
    : _position(glm::vec3(posX, posY, posZ))
    , _front(glm::vec3(0.0F, 0.0F, -1.0F))
    , _worldUp(glm::vec3(upX, upY, upZ))
    , _yaw(yaw)
    , _pitch(pitch)
    , _moveSpeed(SPEED)
    , _mouseSen(SEN)
    , _zoom(ZOOM) {
    updateCameraVectors();
}

const glm::vec3& Camera::getPosition() const noexcept {
    return _position;
}

void Camera::setPosition(const glm::vec3& position) noexcept {
    _position = position;
}

const glm::vec3& Camera::getFront() const noexcept {
    return _front;
}

const glm::vec3& Camera::getUp() const noexcept {
    return _up;
}

const glm::vec3& Camera::getRight() const noexcept {
    return _right;
}

GLfloat Camera::getYaw() const noexcept {
    return _yaw;
}

void Camera::setYaw(GLfloat yaw) noexcept {
    _yaw = yaw;
}

GLfloat Camera::getPitch() const noexcept {
    return _pitch;
}

void Camera::setPitch(GLfloat pitch) noexcept {
    _pitch = fminf(pitch, 89.0F);
    _pitch = fmaxf(_pitch, -89.0F);
}

GLfloat Camera::getMoveSpeed() const noexcept {
    return _moveSpeed;
}

void Camera::setMoveSpeed(GLfloat moveSpeed) noexcept {
    _moveSpeed = moveSpeed;
}

GLfloat Camera::getMouseSen() const noexcept {
    return _mouseSen;
}

void Camera::setMouseSen(GLfloat mouseSen) noexcept {
    _mouseSen = mouseSen;
}

GLfloat Camera::getZoom() const noexcept {
    return _zoom;
}

void Camera::setZoom(GLfloat zoom) noexcept {
    _zoom = fmaxf(zoom, 1.0F);
    _zoom = fminf(_zoom, 45.0F);
}

void Camera::keyboard(Camera::Movement direction, GLfloat deltaTime) noexcept {
    GLfloat velocity = _moveSpeed * deltaTime;
    if (direction == FRONT) _position += _front * velocity;
    if (direction == BACK) _position -= _front * velocity;
    if (direction == LEFT) _position -= _right * velocity;
    if (direction == RIGHT) _position += _right * velocity;
}

void Camera::mouseMove(GLfloat xoffset, GLfloat yoffset, GLboolean constrainPitch) noexcept {
    xoffset *= _mouseSen;
    yoffset *= _mouseSen;

    _yaw += xoffset;
    _pitch += yoffset;

    // make sure that when pitch is out of bounds, screen doesn't get flipped
    if (constrainPitch) {
        _pitch = fminf(_pitch, 89.0F);
        _pitch = fmaxf(_pitch, -89.0F);
    }

    updateCameraVectors();
}

void Camera::mouseScroll(GLfloat yoffset) noexcept {
    _zoom -= yoffset;
    _zoom = fmaxf(_zoom, 1.0F);
    _zoom = fminf(_zoom, 45.0F);
}

glm::mat4 Camera::getView() noexcept {
    return glm::lookAt(_position, _position + _front, _up);
}

void Camera::update() noexcept {
    updateCameraVectors();
}
}   // namespace skl::opengl
