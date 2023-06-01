#include "renderer/camera.h"

#include <cmath>
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include "core/input.h"

// initial values
constexpr glm::vec3 g_CameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
constexpr glm::vec3 g_CameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
constexpr glm::vec3 g_CameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
constexpr glm::vec3 g_Target = glm::vec3(0.0f, 0.0f, 0.0f);
constexpr float g_Yaw = -90.0f;
constexpr float g_Pitch = 0.0f;
constexpr float g_FOVy = glm::radians(45.0f);
constexpr float g_ZNear = 0.1f;
constexpr float g_ZFar = 50.0f;


Camera::Camera(float aspectRatio)
	: m_AspectRatio{ aspectRatio },
	  m_CameraPos{ g_CameraPos },
	  m_CameraFront{ g_CameraFront },
	  m_CameraUp{ g_CameraUp },
	  m_Target{ g_Target },
	  m_Yaw{ g_Yaw },
	  m_Pitch{ g_Pitch },
	  m_LastX{ 0.0f },
	  m_LastY{ 0.0f },
	  m_FOVy{ g_FOVy },
	  m_ZNear{ g_ZNear },
	  m_ZFar{ g_ZFar },
	  m_ViewMatrix{},
	  m_ProjectionMatrix{}
{}

void Camera::OnUpdate(float deltatime)
{
	m_ViewMatrix = glm::lookAt(m_CameraPos, m_CameraPos + m_CameraFront, m_CameraUp);
	m_ProjectionMatrix = glm::perspective(m_FOVy, m_AspectRatio, m_ZNear, m_ZFar);
	// glm was designed for opengl where the y-coord for clip coordinate is flipped
	m_ProjectionMatrix[1][1] *= -1;

	// movement
	const float cameraSpeed = 3.0f * deltatime;
	if (Input::IsKeyPressed(Key::W)) // forward
		m_CameraPos += cameraSpeed * m_CameraFront;
	else if (Input::IsKeyPressed(Key::S)) // backward
		m_CameraPos -= cameraSpeed * m_CameraFront;

	if (Input::IsKeyPressed(Key::A)) // left
		m_CameraPos -= cameraSpeed * (glm::normalize(glm::cross(m_CameraFront, m_CameraUp)));
	else if (Input::IsKeyPressed(Key::D)) // right
		m_CameraPos += cameraSpeed * (glm::normalize(glm::cross(m_CameraFront, m_CameraUp)));

	if (Input::IsKeyPressed(Key::E)) // up
	{
		const glm::vec3 rightVec = glm::cross(m_CameraFront, m_CameraUp);
		const glm::vec3 upVec = glm::cross(rightVec, m_CameraFront);
		m_CameraPos += cameraSpeed * glm::normalize(upVec);
	}
	else if (Input::IsKeyPressed(Key::Q)) // down
	{
		const glm::vec3 rightVec = glm::cross(m_CameraFront, m_CameraUp);
		const glm::vec3 upVec = glm::cross(rightVec, m_CameraFront);
		m_CameraPos -= cameraSpeed * glm::normalize(upVec);
	}

	// reset camera
	if (Input::IsKeyPressed(Key::R))
	{
		m_FirstMouseMove = true;
		m_CameraPos = g_CameraPos;
		m_CameraFront = g_CameraFront;
		m_CameraUp = g_CameraUp;
		m_Target = g_Target;
		m_Yaw = g_Yaw;
		m_Pitch = g_Pitch;
		m_LastX = 0.0f;
		m_LastY = 0.0f;
	}
}

void Camera::OnMouseMove(double xpos, double ypos)
{
	// initial values: m_Yaw = -90.0f, m_Pitch = 0.0f
	if (!Input::IsMouseButtonPressed(Mouse::BUTTON_1)) // only move the camera on mouse button click
	{
		m_FirstMouseMove = true;
		return;
	}

	if (m_FirstMouseMove)
	{
		m_LastX = static_cast<float>(xpos);
		m_LastY = static_cast<float>(ypos);
		m_FirstMouseMove = false;
	}

	const float sensitivity = 0.1f;
	float xOffset = (static_cast<float>(xpos) - m_LastX) * sensitivity;
	float yOffset = (static_cast<float>(ypos) - m_LastY) * sensitivity;

	m_LastX = static_cast<float>(xpos);
	m_LastY = static_cast<float>(ypos);

	m_Yaw += xOffset;
	m_Pitch -= yOffset; // negative because the y coord is flupped in projection matrix

	if (m_Pitch > 89.0f)
		m_Pitch = 89.0f;
	if (m_Pitch < -89.0f)
		m_Pitch = -89.0f;

	if (m_Yaw > 359.0f || m_Yaw < -359.0f)
		m_Yaw = 0.0f;

	glm::vec3 direction;
	direction.x = std::cosf(glm::radians(m_Yaw)) * std::cosf(glm::radians(m_Pitch));
	direction.y = std::sinf(glm::radians(m_Pitch));
	direction.z = std::sinf(glm::radians(m_Yaw)) * std::cosf(glm::radians(m_Pitch));
	m_CameraFront = glm::normalize(direction);
}
