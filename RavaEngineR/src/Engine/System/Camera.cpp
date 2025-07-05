#include "rvpch.h"

#include "Engine/System/Camera.h"
// #include "Framework/Timestep.h"

namespace RV {
Camera::Camera() {
	SetPerspectiveProjection(glm::radians(50.f), _aspect, 0.1f, 100.f);
}

void Camera::RecalculateProjection() {
	if (_projectionType == ProjectionType::Perspective) {
		SetPerspectiveProjection(_perspectiveFOV, _aspect, _perspectiveNear, _perspectiveFar);
	} else {
		float orthoLeft   = -_orthographicSize * _aspect * 0.5f;
		float orthoRight  = _orthographicSize * _aspect * 0.5f;
		float orthoTop    = _orthographicSize * 0.5f;
		float orthoBottom = -_orthographicSize * 0.5f;

		SetOrthographicProjection3D(orthoLeft, orthoRight, orthoTop, orthoBottom, _orthographicNear, _orthographicFar);
	}
}

void Camera::UpdateView() {
	if (!_initialized) {
		return;
	}
	//_position = glm::mix(_position, _targetPosition, _positionSmoothness * Timestep::Count());
	//_rotation = glm::mix(_rotation, _targetRotation, _rotationSmoothness * Timestep::Count());

	SetViewYXZ(_position, _rotation);
}

void Camera::MoveCamera(glm::vec3 position, glm::vec3 rotation) {
	if (_isSmoothTranslate) {
		SetTargetViewYXZ(position, rotation);
		UpdateView();
	} else {
		SetViewYXZ(position, rotation);
	}
}

void Camera::SetProjectionType(ProjectionType type) {
	_projectionType = type;
	RecalculateProjection();
}

void Camera::SetPerspectiveProjection(float fovy, float aspect, float zNear, float zFar) {
	_perspectiveFOV  = fovy;
	_aspect          = aspect;
	_perspectiveNear = zNear;
	_perspectiveFar  = zFar;

	ENGINE_ASSERT(glm::abs(aspect - std::numeric_limits<float>::epsilon()) > 0.0f);
	_projectionMatrix = glm::perspective(_perspectiveFOV, _aspect, _perspectiveNear, _perspectiveFar);
}

void Camera::SetOrthographicProjection(float left, float right, float top, float bottom, float zNear, float zFar) {
	_projectionMatrix       = glm::mat4{1.0f};
	_projectionMatrix[0][0] = 2.f / (right - left);
	_projectionMatrix[1][1] = 2.f / (bottom - top);
	_projectionMatrix[2][2] = 1.f / (zFar - zNear);
	_projectionMatrix[3][0] = -(right + left) / (right - left);
	_projectionMatrix[3][1] = -(bottom + top) / (bottom - top);
	_projectionMatrix[3][2] = -zNear / (zFar - zNear);
}

void Camera::SetOrthographicProjection3D(float left, float right, float top, float bottom, float zNear, float zFar) {
	_projectionMatrix = glm::ortho(-left, -right, bottom, top, zNear, zFar);
}

void Camera::SetViewTarget(glm::vec3 position, glm::vec3 target, glm::vec3 up) {
	_viewMatrix        = glm::lookAt(position, target, up);
	_inverseViewMatrix = glm::inverse(_viewMatrix);
}

void Camera::SetViewYXZ(glm::vec3 position, glm::vec3 rotation) {
	const float c3 = glm::cos(rotation.z);
	const float s3 = glm::sin(rotation.z);
	const float c2 = glm::cos(rotation.x);
	const float s2 = glm::sin(rotation.x);
	const float c1 = glm::cos(rotation.y);
	const float s1 = glm::sin(rotation.y);
	const glm::vec3 u{(c1 * c3 + s1 * s2 * s3), (c2 * s3), (c1 * s2 * s3 - c3 * s1)};
	const glm::vec3 v{(c3 * s1 * s2 - c1 * s3), (c2 * c3), (c1 * c3 * s2 + s1 * s3)};
	const glm::vec3 w{(c2 * s1), (-s2), (c1 * c2)};
	_viewMatrix       = glm::mat4{1.f};
	_viewMatrix[0][0] = u.x;
	_viewMatrix[1][0] = u.y;
	_viewMatrix[2][0] = u.z;
	_viewMatrix[0][1] = v.x;
	_viewMatrix[1][1] = v.y;
	_viewMatrix[2][1] = v.z;
	_viewMatrix[0][2] = w.x;
	_viewMatrix[1][2] = w.y;
	_viewMatrix[2][2] = w.z;
	_viewMatrix[3][0] = -glm::dot(u, position);
	_viewMatrix[3][1] = -glm::dot(v, position);
	_viewMatrix[3][2] = -glm::dot(w, position);

	_inverseViewMatrix       = glm::mat4{1.f};
	_inverseViewMatrix[0][0] = u.x;
	_inverseViewMatrix[0][1] = u.y;
	_inverseViewMatrix[0][2] = u.z;
	_inverseViewMatrix[1][0] = v.x;
	_inverseViewMatrix[1][1] = v.y;
	_inverseViewMatrix[1][2] = v.z;
	_inverseViewMatrix[2][0] = w.x;
	_inverseViewMatrix[2][1] = w.y;
	_inverseViewMatrix[2][2] = w.z;
	_inverseViewMatrix[3][0] = position.x;
	_inverseViewMatrix[3][1] = position.y;
	_inverseViewMatrix[3][2] = position.z;
}

void Camera::SetTargetViewYXZ(glm::vec3 targetPosition, glm::vec3 targetRotation) {
	if (!_initialized) {
		_position    = targetPosition;
		_rotation    = targetRotation;
		_initialized = true;
	}

	_targetPosition = targetPosition;
	_targetRotation = targetRotation;
}
}  // namespace RV
