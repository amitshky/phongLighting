#pragma once

#include <cstdint>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>


class Camera
{
public:
	Camera(float aspectRatio);

	void OnUpdate(float deltatime);
	void OnMouseMove(double xpos, double ypos);

	inline void SetAspectRatio(float aspectRatio) { m_AspectRatio = aspectRatio; }
	inline glm::mat4 GetViewMatrix() const { return m_ViewMatrix; }
	inline glm::mat4 GetProjectionMatrix() const { return m_ProjectionMatrix; }
	inline glm::vec3 GetCameraPosition() const { return m_CameraPos; }

private:
	bool m_FirstMouseMove = true;
	float m_AspectRatio;

	glm::vec3 m_CameraPos;
	glm::vec3 m_CameraFront;
	glm::vec3 m_CameraUp;
	glm::vec3 m_Target;

	float m_Yaw;
	float m_Pitch;

	float m_LastX;
	float m_LastY;

	// projection matrix
	float m_FOVy;
	float m_ZNear;
	float m_ZFar;

	glm::mat4 m_ViewMatrix;
	glm::mat4 m_ProjectionMatrix;
};