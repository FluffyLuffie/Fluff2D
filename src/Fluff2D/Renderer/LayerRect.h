#pragma once

#include <iostream>

class LayerRect
{
public:
	enum class LayerType : int { image = 0, folder = 1, divider = 2 };

	//from psd
	int x, y, w, h;
	std::string layerName = "";
	LayerType layerType = LayerType::image;

	LayerRect();
	//LayerRect(int _x, int _y, int _w, int _h);
	LayerRect(const LayerRect& layerRect);

	~LayerRect() {}
};

