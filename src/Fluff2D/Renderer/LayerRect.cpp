#include "LayerRect.h"

LayerRect::LayerRect()
{
	y = x = w = h = 0;
}

LayerRect::LayerRect(const LayerRect& layerRect)
{
	x = layerRect.x;
	y = layerRect.y;
	w = layerRect.w;
	h = layerRect.h;
}
