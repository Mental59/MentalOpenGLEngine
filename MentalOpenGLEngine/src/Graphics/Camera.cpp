#include "Camera.h"
#include <glm/gtc/matrix_transform.hpp>
#include "Time.h"

Camera::Camera(
	const glm::vec3& cameraPos,
	float movementSpeed,
	float mouseSensitivity
) :
	mCameraPos(cameraPos),
	mMovementSpeed(movementSpeed),
	mMouseSensitivity(mouseSensitivity),
	mYaw(-90.0f), mPitch(0.0f), mZoom(45.0f)
{
	UpdateCameraBasisVectors();
}

glm::mat4 Camera::GetViewMatrix() const
{
	return glm::lookAt(mCameraPos, mCameraPos + mCameraForward, mCameraUp);
}

void Camera::Move(Movement movement)
{
	float velocity = mMovementSpeed * Time::DeltaTime;

	switch (movement)
	{
	case Camera::Right:
		mCameraPos += mCameraRight * velocity;
		break;
	case Camera::Left:
		mCameraPos -= mCameraRight * velocity;
		break;
	case Camera::Forward:
		mCameraPos += mCameraForward * velocity;
		break;
	case Camera::Backward:
		mCameraPos -= mCameraForward * velocity;
		break;
	case Camera::Up:
		mCameraPos += mCameraUp * velocity;
		break;
	case Camera::Down:
		mCameraPos -= mCameraUp * velocity;
		break;
	}
}

void Camera::Rotate(float xOffset, float yOffset)
{
	mYaw += xOffset * mMouseSensitivity * Time::DeltaTime;
	mPitch += yOffset * mMouseSensitivity * Time::DeltaTime;

	if (mPitch > 89.0f)
	{
		mPitch = 89.0f;
	}
	if (mPitch < -89.0f)
	{
		mPitch = -89.0f;
	}

	UpdateCameraBasisVectors();
}

void Camera::Zoom(float yOffset)
{
	mZoom -= yOffset * Time::DeltaTime * 100.0f;

	if (mZoom < 1.0f)
	{
		mZoom = 1.0f;
	}
	if (mZoom > 45.0f)
	{
		mZoom = 45.0f;
	}
}

void Camera::UpdateCameraBasisVectors()
{
	glm::vec3 forward(
		cos(glm::radians(mYaw)) * cos(glm::radians(mPitch)),
		sin(glm::radians(mPitch)),
		sin(glm::radians(mYaw)) * cos(glm::radians(mPitch))
	);

	mCameraForward = glm::normalize(forward);
	mCameraRight = glm::normalize(glm::cross(mCameraForward, glm::vec3(0.0f, 1.0f, 0.0f)));
	mCameraUp = glm::cross(mCameraRight, mCameraForward);
}
