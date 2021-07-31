#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <thread>
#include <glad/glad.h>

#include "Model.h"
#include "LayerRect.h"
#include "../UI/ModelPartUI.h"
#include "../Core/Window.h"

#include "stb_image/stb_image_write.h"
#include "rectpack2D/finders_interface.h"
#include "alpha-bleeding/alphaBleeding.h"
#include "threadPool/ThreadPool.h"

using spaces_type = rectpack2D::empty_spaces<true, rectpack2D::default_empty_spaces>;
using rect_type = rectpack2D::output_rect_t<spaces_type>;

class TextureLoader
{
public:
	static void loadTexture(unsigned int* texture, const char* fileName, int* width, int* height, int* nrChannels);
	static void loadPsdFile(const char* fileName, std::shared_ptr<Model> model);

private:
	static std::vector<rect_type> prepareTextureAtlas(std::vector <LayerRect>& layerRects, int texturePixelBuffer, int *atlasWidth, int *atlasHeight);
	static void bleedPng(unsigned char* data, int width, int height);

	static int nextPower2(int num);
};

