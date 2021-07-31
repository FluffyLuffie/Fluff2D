#pragma once

#include <glm/glm.hpp>

class Settings
{
public:
	inline static glm::vec3 backgroundColor = glm::vec3(0.75f);

	inline static int canvasLineWidth = 1;
	inline static glm::vec3 canvasBorderColor = glm::vec3(0.0f);

	inline static int meshPointSize = 5;
	inline static glm::vec3 meshPointColor = glm::vec3(1.0f, 1.0f, 1.0f);
	inline static glm::vec3 meshPointHighlightColor = glm::vec3(0.0f, 1.0f, 0.0f);
	inline static glm::vec3 meshPointSelectedColor = glm::vec3(1.0f, 1.0f, 0.0f);

	inline static int meshPointBorderSize = 2;
	inline static glm::vec3 meshPointBorderColor = glm::vec3(0.0f, 0.0f, 0.0f);

	inline static float vertexDetectionDistance = 30.0f;

	inline static int meshLineWidth = 2;
	inline static glm::vec3 meshLineColor = glm::vec3(1.0f, 0.0f, 0.0f);
};

