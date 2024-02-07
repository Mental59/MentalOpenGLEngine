#pragma once

#include <glm/glm.hpp>

class Camera
{
public:
	enum Movement
	{
		Right, Left, Forward, Backward, Up, Down
	};

	Camera(
		const glm::vec3& cameraPos,
		float movementSpeed,
		float mouseSensitivity
	);

	glm::mat4 GetViewMatrix() const;
	inline float GetZoom() const { return mZoom; }
	inline const glm::vec3& GetWorldPosition() const { return mCameraPos; }
	inline const glm::vec3& GetForwardDirection() const { return mCameraForward; }
	inline const glm::vec3& GetRightDirection() const { return mCameraRight; }
	inline const glm::vec3& GetUpDirection() const { return mCameraUp; }

	void Move(Movement movement);
	void Rotate(float xOffset, float yOffset);
	void Zoom(float yOffset);

private:
	void UpdateCameraBasisVectors();

	glm::vec3 mCameraPos;
	glm::vec3 mCameraForward;
	glm::vec3 mCameraRight;
	glm::vec3 mCameraUp;

	float mMovementSpeed, mMouseSensitivity;
	float mYaw, mPitch, mZoom;
};