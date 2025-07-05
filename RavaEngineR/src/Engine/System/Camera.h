#pragma once

namespace RV {
class Camera {
   public:
	enum class ProjectionType {
		Perspective  = 0,
		Orthographic = 1
	};

   public:
	Camera();
	~Camera() = default;

	void RecalculateProjection();
	void UpdateView();
	void MoveCamera(glm::vec3 position, glm::vec3 rotation);

	void SetProjectionType(ProjectionType type);
	void SetPerspectiveProjection(float fovy, float aspect, float zNear, float zFar);
	void SetOrthographicProjection(float left, float right, float top, float bottom, float zNear, float zFar);
	void SetOrthographicProjection3D(float left, float right, float top, float bottom, float zNear, float zFar);
	void SetViewTarget(glm::vec3 position, glm::vec3 target, glm::vec3 up = glm::vec3{0.f, 1.f, 0.f});
	void SetViewYXZ(glm::vec3 position, glm::vec3 rotation);
	void SetTargetViewYXZ(glm::vec3 targetPosition, glm::vec3 targetRotation);
	void SetPerspectiveVerticalFOV(float fov) { _perspectiveFOV = fov; }
	void SetPerspectiveNearClip(float zNear) { _perspectiveFOV = zNear; }
	void SetPerspectiveFarClip(float zFar) { _perspectiveFOV = zFar; }
	void SetOrthographicSize(float size) { _orthographicSize = size; }
	void SetOrthographicNearClip(float zNear) { _orthographicNear = zNear; }
	void SetOrthographicFarClip(float zFar) { _orthographicFar = zFar; }
	void SetSmoothTranslate(bool isSmooth) { _isSmoothTranslate = isSmooth; }

	float EaseInOut(float t) { return t < 0.5f ? 2.0f * t * t : -1.0f + (4.0f - 2.0f * t) * t; }
	ProjectionType GetProjectionType() const { return _projectionType; }
	float GetPerspectiveVerticalFOV() const { return _perspectiveFOV; }
	float GetPerspectiveNearClip() const { return _perspectiveNear; }
	float GetPerspectiveFarClip() const { return _perspectiveFar; }
	float GetOrthographicSize() const { return _orthographicSize; }
	float GetOrthographicNearClip() const { return _orthographicNear; }
	float GetOrthographicFarClip() const { return _orthographicFar; }
	bool GetIsSmoothTranslate() const { return _isSmoothTranslate; }
	const glm::mat4& GetProjection() const { return _projectionMatrix; }
	const glm::mat4& GetView() const { return _viewMatrix; }
	const glm::mat4& GetInverseView() const { return _inverseViewMatrix; }
	const glm::vec3 GetPosition() const { return glm::vec3(_viewMatrix[3]); }

   private:
	ProjectionType _projectionType = ProjectionType::Perspective;
	glm::mat4 _projectionMatrix{1.f};
	glm::mat4 _viewMatrix{1.f};
	glm::mat4 _inverseViewMatrix{1.f};

	float _aspect = 16.0f / 9.0f;

	float _perspectiveFOV  = glm::radians(45.0f);
	float _perspectiveNear = 0.01f;
	float _perspectiveFar  = 1000.0f;

	float _orthographicSize = 10.0f;
	float _orthographicNear = -1.0f;
	float _orthographicFar  = 1.0f;

	float _zNear      = 0.01f;
	float _zFar       = 100.0f;
	float _zoomFactor = 1.0f;

	bool _isSmoothTranslate = true;
	glm::vec3 _position{0.0f};          // Current position
	glm::vec3 _rotation{0.0f};          // Current rotation
	glm::vec3 _targetPosition{0.0f};    // Target position
	glm::vec3 _targetRotation{0.0f};    // Target rotation
	float _positionSmoothness = 3.0f;   // Higher value = faster movement
	float _rotationSmoothness = 10.0f;  // Higher value = faster rotation
	bool _initialized         = false;
};
}  // namespace RV