#pragma once

#include <vector>
#include <GLFW/glfw3.h>

class AlphaBleeding
{
public:
	/*
	things I changed

	changed to only sample the 4 adjacent pixels, ignore the corners
	limit how many times to loop, only need to do it for the extra size of the rect
	casting stuff to fix warning messages
	random comments to help me understand it a bit more later
	*/
	static void bleedPng(unsigned char* image, int width, int height, int maxLoop)
	{
		const int N = width * height;

		std::vector<unsigned char> opaque(N);
		std::vector<bool>   loose(N);
		std::vector<size_t> pending;
		std::vector<size_t> pendingNext;

		pending.reserve(N);
		pendingNext.reserve(N);

		int offsets[][2] = {
			{ 0, -1},
			{-1,  0},
			{ 1,  0},
			{ 0,  1},
		};

		for (size_t i = 0, j = 3; i < N; i++, j += 4)
		{
			if (image[j] == 0)
			{
				bool isLoose = true;

				int x = static_cast<int>(i % width);
				int y = static_cast<int>(i / width);

				for (char k = 0; k < 4; k++)
				{
					int s = offsets[k][0];
					int t = offsets[k][1];

					//check if all 4 adjacent pixels are also transparent
					//if so then loose, pending if a non transparent pixel exists
					if (x + s >= 0 && x + s < width && y + t >= 0 && y + t < height)
					{
						size_t index = j + 4 * (s + t * width);

						if (image[index] != 0)
						{
							isLoose = false;
							break;
						}
					}
				}

				if (!isLoose)
					pending.push_back(i);
				else
					loose[i] = true;
			}
			else
			{
				opaque[i] = -1;
			}
		}

		while (pending.size() > 0 && maxLoop)
		{
			maxLoop--;

			pendingNext.clear();

			for (size_t p = 0; p < pending.size(); p++)
			{
				//i is the index of the i-th pixel
				//j is the pixel number
				size_t i = pending[p] * 4;
				size_t j = pending[p];

				int x = static_cast<int>(j % width);
				int y = static_cast<int>(j / width);

				int r = 0;
				int g = 0;
				int b = 0;

				int count = 0;

				for (char k = 0; k < 4; k++)
				{
					int s = offsets[k][0];
					int t = offsets[k][1];

					if (x + s >= 0 && x + s < width && y + t >= 0 && y + t < height)
					{
						t *= width;

						if (opaque[j + s + t] & 1)
						{
							size_t index = i + 4 * (s + t);

							r += image[index + 0];
							g += image[index + 1];
							b += image[index + 2];

							count++;
						}
					}
				}

				if (count > 0)
				{
					image[i + 0] = r / count;
					image[i + 1] = g / count;
					image[i + 2] = b / count;

					opaque[j] = 0xFE;

					for (char k = 0; k < 4; k++)
					{
						int s = offsets[k][0];
						int t = offsets[k][1];

						if (x + s >= 0 && x + s < width && y + t >= 0 && y + t < height)
						{
							size_t index = j + s + t * width;

							if (loose[index])
							{
								pendingNext.push_back(index);
								loose[index] = false;
							}
						}
					}
				}
				else
				{
					pendingNext.push_back(j);
				}
			}

			if (pendingNext.size() > 0)
			{
				for (size_t p = 0; p < pending.size(); p++)
					opaque[pending[p]] >>= 1;
			}

			pending.swap(pendingNext);
		}
	}
};