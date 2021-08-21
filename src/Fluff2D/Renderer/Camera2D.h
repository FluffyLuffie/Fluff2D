#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "../Renderer/Shader.h"
#include "../Core/Window.h"

class Camera2D
{
public:
	inline static glm::vec2 pos = glm::vec2(0.0f);
	inline static float scale = 1.0f;
	inline static float rotation = 0.0f;

	inline static glm::mat4 projection = glm::mat4(1.0f);

	static void update();
};

