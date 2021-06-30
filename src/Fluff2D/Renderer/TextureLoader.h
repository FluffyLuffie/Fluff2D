#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <glad/glad.h>

#include "stb_image.h"
#include "stb_image_write.h"

class TextureLoader
{
public:
	static void loadTexture(unsigned int* texture, const char* fileName, int* width, int* height, int* nrChannels);
	static void loadPsdFile(const char* fileName);

private:
	enum class LayerType : int {image = 0, folder = 1, divider = 2};

	struct LayerRect
	{
		//from psd
		int top, left, bottom, right;
		std::string layerName = "";
		TextureLoader::LayerType layerType = TextureLoader::LayerType::image;
		std::vector <char> imageBytes;

		//to be used in making texture atlas
		int width, height;
		int posX = 0, posY = 0;

		LayerRect()
		{
			top = left = bottom = right = width = height = 0;
		}

		LayerRect(int _top, int _left, int _bottom, int _right)
		{
			top = _top;
			left = _left;
			bottom = _bottom;
			right = _right;

			width = right = left;
			height = bottom - top;
		}

		~LayerRect() {}

		void calculateDimensions()
		{
			width = right - left;
			height = bottom - top;
		}
	};

	static void createTextureAtlas(std::vector <LayerRect> &layerRects);
};

