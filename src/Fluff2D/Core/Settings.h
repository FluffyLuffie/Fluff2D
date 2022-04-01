#pragma once

#include "../UI/Localization.h"

#include <string>
#include <glm/glm.hpp>

class Settings
{
public:
	inline static bool renderMeshData = true;
	inline static bool renderDeformerData = true;

	inline static Localization::Language selectedLanguage = Localization::Language::English;

	inline static int fontSize = 15;
	inline static std::string fontFile = "C:/Windows/Fonts/Meiryob.ttc";

	inline static glm::vec3 backgroundColor = glm::vec3(0.75f);

	inline static bool showCanvas = true;
	inline static int canvasLineWidth = 1;
	inline static glm::vec3 canvasBorderColor = glm::vec3(0.0f);

	inline static int meshPointSize = 2;
	inline static glm::vec3 meshPointColor = glm::vec3(1.0f, 1.0f, 1.0f);
	inline static glm::vec3 meshPointHighlightColor = glm::vec3(1.0f, 0.0f, 0.0f);
	inline static glm::vec3 meshPointSelectedColor = glm::vec3(1.0f, 0.0f, 0.0f);

	inline static int meshPointBorderSize = 2;
	inline static glm::vec3 meshPointBorderColor = glm::vec3(0.0f, 0.0f, 0.0f);

	inline static int meshLineWidth = 1;
	inline static glm::vec3 meshLineColor = glm::vec3(0.2f, 0.2f, 0.2f);
	inline static glm::vec3 meshHighlightColor = glm::vec3(1.0f, 0.0f, 0.0f);

	inline static glm::vec3 warpDeformerColor = glm::vec3(0.0f, 0.8f, 0.0f);
	inline static glm::vec3 rotationDeformerColor = glm::vec3(0.0f, 0.0f, 0.8f);
	inline static int rotationDeformerWidth = 3;

	inline static float vertexDetectionDistance = 20.0f;
};

