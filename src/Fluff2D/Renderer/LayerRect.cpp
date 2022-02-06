#include "LayerRect.h"

LayerRect::LayerRect()
{
	y = x = w = h = 0;
}

/*
LayerRect::LayerRect(int _x, int _y, int _w, int _h)
{
	x = _x;
	y = _y;
	w = _w;
	h = _h;
}
*/

LayerRect::LayerRect(const LayerRect& layerRect)
{
	x = layerRect.x;
	y = layerRect.y;
	w = layerRect.w;
	h = layerRect.h;
}
