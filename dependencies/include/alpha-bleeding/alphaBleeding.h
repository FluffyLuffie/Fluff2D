#pragma once

#include <vector>
#include <GLFW/glfw3.h>

class AlphaBleeding
{
public:
	/*
	things I changed

	changed to only sample the 4 adjacent pixels, ignore the corners
	added premultiplied alpha
	limit how many times to loop, only need to do it for the extra size of the rect
	casting stuff to fix warning messages
	random comments to help me understand it a bit more later
	*/
	static void bleedPng(unsigned char* image, int width, int height, int maxLoop)
	{
		const int N = width * height;

		for (size_t i = 0, j = 3; i < N; i++, j += 4)
		{
			//premultiply alpha
			image[j - 1] = (unsigned char)(image[j - 1] * (image[j] / 255.0f));
			image[j - 2] = (unsigned char)(image[j - 2] * (image[j] / 255.0f));
			image[j - 3] = (unsigned char)(image[j - 3] * (image[j] / 255.0f));
		}
	}
};