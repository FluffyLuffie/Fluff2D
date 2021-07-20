#include "TextureLoader.h"

void TextureLoader::loadTexture(unsigned int* texture, const char* fileName, int* width, int* height, int* nrChannels)
{
	glGenTextures(1, texture);
	glBindTexture(GL_TEXTURE_2D, *texture);

	float borderColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	unsigned char *data = stbi_load(fileName, width, height, nrChannels, 0);
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, *width, *height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture\n";
	}
	stbi_image_free(data);
}

//I only have Clip Studio Paint so idk if it works in Photoshop
//Very high chance of breaking if you so something besides just plain raster layers and folders
//Read this if you want to understand a fraction of the pain I went for you, and others if this becomes an open source project like I intended to
//https://www.adobe.com/devnet-apps/photoshop/fileformatashtml/
void TextureLoader::loadPsdFile(const char* fileName, Model *model)
{
	//might move this to a class variable later
	std::vector <LayerRect> layerRects;

	std::cout << "Opening " << fileName << std::endl;

	//check if file successfully opened
	std::ifstream pf (fileName, std::ios::binary);
	if (!pf.is_open())
	{
		std::cout << "Error opening psd file" << std::endl;
		return;
	}

	char buffer[16];
	pf.seekg(0, std::ios::beg);

#pragma region File Header Section
	//check signature
	pf.read(buffer, 4);
	if (std::strncmp(buffer, "8BPS", 4) != 0)
	{
		std::cout << "PSD file has incorrect signature" << std::endl;
		return;
	}

	//check version
	pf.read(buffer, 2);
	if (buffer[0] != 0 || buffer[1] != 1)
	{
		std::cout << "PSD file is incorrect version" << std::endl;
		return;
	}

	//skip over junk
	pf.seekg(8, std::ios::cur);

	//get width and height of psd file
	pf.read(buffer, 4);
	int psdHeight = (buffer[0] << 24) | ((buffer[1] & 0xFF) << 16) | ((buffer[2] & 0xFF) << 8) | (buffer[3] & 0xFF);
	pf.read(buffer, 4);
	int psdWidth = (buffer[0] << 24) | ((buffer[1] & 0xFF) << 16) | ((buffer[2] & 0xFF) << 8) | (buffer[3] & 0xFF);
	std::cout << "PSD dimensions: " << psdWidth << "x" << psdHeight << std::endl;

	//skip over junk
	pf.seekg(4, std::ios::cur);
#pragma endregion

#pragma region Color Mode Data Section
	//not important, assume user isn't on index or duotone mode
	pf.seekg(4, std::ios::cur);
#pragma endregion

#pragma region Image Resources Section
	//get image resource length
	pf.read(buffer, 4);

	//skip over whatever junk of a section this is
	pf.seekg((buffer[0] << 24) | ((buffer[1] & 0xFF) << 16) | ((buffer[2] & 0xFF) << 8) | (buffer[3] & 0xFF), std::ios::cur);
#pragma endregion

#pragma region Layer and Mask Information Section, also make texture atlas
	//If anything goes wrong, 99% sure it's from this section, especially the Channel Image Data Section

	//layer info
	//skip junk data
	pf.seekg(8, std::ios::cur);

	//layers count as 1, folders count as 2, don't ask me why cause that's how psd works
	pf.read(buffer, 2);
	int layerCount = std::abs(((buffer[0] & 0xFF) << 8) | (buffer[1] & 0xFF));
	layerRects.resize(layerCount);

	//set up root part
	model->rootPart.children.reserve(layerCount);
	ModelPartUI* currentFolder = &model->rootPart;

	size_t totalLayerPixels = 0;

	//read in layer data
	for (int layerNum = 0; layerNum < layerCount; layerNum++)
	{
		pf.read(buffer, 16);
		layerRects[layerNum].y = (buffer[0] << 24) | ((buffer[1] & 0xFF) << 16) | ((buffer[2] & 0xFF) << 8) | (buffer[3] & 0xFF);
		layerRects[layerNum].x = (buffer[4] << 24) | ((buffer[5] & 0xFF) << 16) | ((buffer[6] & 0xFF) << 8) | (buffer[7] & 0xFF);
		layerRects[layerNum].h = ((buffer[8] << 24) | ((buffer[9] & 0xFF) << 16) | ((buffer[10] & 0xFF) << 8) | (buffer[11] & 0xFF)) - layerRects[layerNum].y;
		layerRects[layerNum].w = ((buffer[12] << 24) | ((buffer[13] & 0xFF) << 16) | ((buffer[14] & 0xFF) << 8) | (buffer[15] & 0xFF)) - layerRects[layerNum].x;

		totalLayerPixels += layerRects[layerNum].h * layerRects[layerNum].w;

		//number of channels
		pf.read(buffer, 2);
		int channels = std::abs(((buffer[0] & 0xFF) << 8) | (buffer[1] & 0xFF));

		//skip over a bunch of stuff cause everything is gibberish
		pf.seekg(channels * 6 + 16, std::ios::cur);

		//layer mask data, nobody cares about you either
		pf.read(buffer, 4);
		pf.seekg((buffer[0] << 24) | ((buffer[1] & 0xFF) << 16) | ((buffer[2] & 0xFF) << 8) | (buffer[3] & 0xFF), std::ios::cur);

		//layer blending ranges, go away too
		pf.seekg(12 + 8 * channels, std::ios::cur);

		//read in layer name
		//whoever designed this part should be thrown in hell
		//if the number of characters is (3 + 4x), for example "arm" or "chicken", the null character does not exist at the end
		bool layerNameBad = false;

		//skip a byte cause idk
		pf.seekg(1, std::ios::cur);

		//read until null or signature
		pf.read(buffer, 1);
		while (buffer[0] != '\0')
		{
			//check if the signature exists "8BIM", might also be "8B64", add later if problems exist
			if (buffer[0] == '8')
			{
				pf.read(&buffer[1], 3);
				if (std::strncmp(&buffer[1], "BIM", 3) == 0)
				{
					layerNameBad = true;
					break;
				}
			}

			layerRects[layerNum].layerName += buffer[0];
			pf.read(buffer, 1);
		}

		//skip through null padding and signature if name isn't created by the devil
		if (!layerNameBad)
		{
			while (buffer[0] == '\0')
			{
				pf.read(buffer, 1);
			}
			pf.seekg(3, std::ios::cur);
		}

		//folder dividers are named like this idk
		if (layerRects[layerNum].layerName.compare("</Layer set>") == 0)
		{
			currentFolder->children.emplace_back(ModelPartUI::PartType::divider);
			currentFolder->children.back().parent = currentFolder;
			currentFolder = &currentFolder->children.back();
			currentFolder->children.reserve(layerCount);

			layerRects[layerNum].layerType = LayerRect::LayerType::divider;

			//key is "lsct", I hope
			//also skip a bunch of extra stuff, number of bytes are wrong on documentation but who cares
			pf.seekg(48, std::ios::cur);

			//skip unicode name
			pf.read(buffer, 4);
			pf.seekg(2 * ((buffer[0] << 24) | ((buffer[1] & 0xFF) << 16) | ((buffer[2] & 0xFF) << 8) | (buffer[3] & 0xFF)), std::ios::cur);

			//some junk I think, hope nothing breaks
			pf.seekg(16, std::ios::cur);
		}
		//if not divider
		else
		{
			//documentation is horrible why is Additional Layer Information section in a random place

			//distinguish if folder or image layer
			pf.read(buffer, 4);
			//if folder
			if (std::strncmp(buffer, "lsct", 4) == 0)
			{
				currentFolder->name = layerRects[layerNum].layerName;
				currentFolder = currentFolder->parent;

				layerRects[layerNum].layerType = LayerRect::LayerType::folder;

				//skip a few things
				pf.seekg(44, std::ios::cur);

				//skip unicode name
				pf.read(buffer, 4);
				pf.seekg(2 * ((buffer[0] << 24) | ((buffer[1] & 0xFF) << 16) | ((buffer[2] & 0xFF) << 8) | (buffer[3] & 0xFF)), std::ios::cur);

				//junk
				pf.seekg(16, std::ios::cur);
			}
			//if image layer
			else
			{
				currentFolder->children.emplace_back(ModelPartUI::PartType::image);
				currentFolder->children.back().name = layerRects[layerNum].layerName;

				//skip a bunch of junk
				pf.read(buffer, 4);
				pf.seekg((buffer[0] << 24) | ((buffer[1] & 0xFF) << 16) | ((buffer[2] & 0xFF) << 8) | (buffer[3] & 0xFF), std::ios::cur);

				//skip over junk
				pf.seekg(12, std::ios::cur);

				//skip unicode name
				pf.read(buffer, 4);
				pf.seekg(2 * ((buffer[0] << 24) | ((buffer[1] & 0xFF) << 16) | ((buffer[2] & 0xFF) << 8) | (buffer[3] & 0xFF)), std::ios::cur);

				//skip more junk
				pf.seekg(16, std::ios::cur);
			}
		}
	}

	//testing new texturePixelBuffer stuff
	int atlasWidth = 0, atlasHeight = 0, tempAtlasWidth = 1, tempAtlasHeight = 1;
	int texturePixelBuffer = nextPower2(static_cast<int>(sqrt(totalLayerPixels))) / 128;
	auto rectangles = prepareTextureAtlas(layerRects, texturePixelBuffer, &tempAtlasWidth, &tempAtlasHeight);

	//loop until new buffer size doesn't change max atlas size
	while (std::max(tempAtlasWidth, tempAtlasHeight) != std::max(atlasWidth, atlasHeight))
	{
		rectangles = prepareTextureAtlas(layerRects, texturePixelBuffer, &tempAtlasWidth, &tempAtlasHeight);

		texturePixelBuffer = std::max(tempAtlasWidth, tempAtlasHeight) / 100;

		rectangles = prepareTextureAtlas(layerRects, texturePixelBuffer, &atlasWidth, &atlasHeight);
	}

	std::vector <unsigned char> atlasBytes;
	atlasBytes.resize(atlasWidth * atlasHeight * 4);

	model->meshList.resize(rectangles.size());

	//Channel Image Data

	//keeps track of how many image layers read
	int imageLayersRead = 0;

	//turn off rectangle flipping for now, implementing it later
	std::vector <std::vector <unsigned char>> layerBytes;
	layerBytes.resize(rectangles.size());
	//idk how many threads to create
	progschj::ThreadPool pool(std::thread::hardware_concurrency() - 1);
	std::vector<std::future<bool>> results;
	unsigned char* texPtr;

	//get rects for each layer
	for (int layerNum = 0; layerNum < layerRects.size(); layerNum++)
	{
		//figure out compression method
		pf.read(buffer, 2);
		int compression = ((buffer[0] & 0xFF) << 8) | (buffer[1] & 0xFF);

		int channelBytesLeft = 0, channelOffset = 0, pixelsRead = 0;

		switch (compression)
		{
		//Raw Compression
		//I'm guessing this only happens in folders so if not... good luck
		case 0:
			pf.seekg(6, std::ios::cur);

			break;
		//RLE Compression
		//only happens in image layers, I hope
		case 1:
			layerBytes[imageLayersRead].resize((layerRects[layerNum].w + texturePixelBuffer * 2) * (layerRects[layerNum].h + texturePixelBuffer * 2) * 4);
			//not done yet
			if (rectangles[imageLayersRead].flipped)
			{
				//channel order is ARGB in psd, RGBA in png
				for (int channel = 0; channel < 4; channel++)
				{
					if (channel == 0)
						channelOffset = 3;
					else
						channelOffset = channel - 1;

					//read each row's byte count
					for (int i = 0; i < layerRects[layerNum].h; i++)
					{
						pf.read(buffer, 2);
						channelBytesLeft += ((buffer[0] & 0xFF) << 8) | (buffer[1] & 0xFF);
					}

					while (channelBytesLeft > 1)
					{
						//read the count
						pf.read(buffer, 1);
						channelBytesLeft--;

						//RLE compressed part
						if ((int)buffer[0] < 0)
						{
							pf.read(buffer + 1, 1);
							channelBytesLeft--;

							for (int i = 0; i < -buffer[0] + 1; i++)
							{
								layerBytes[imageLayersRead][(texturePixelBuffer + pixelsRead / layerRects[layerNum].w +
									+(layerRects[layerNum].w - 1 + texturePixelBuffer - pixelsRead % layerRects[layerNum].w) * (layerRects[layerNum].h + texturePixelBuffer * 2))
									* 4 + channelOffset] = buffer[1];
								pixelsRead++;
							}
						}
						//not RLE compressed part
						else
						{
							for (int i = 0; i < buffer[0] + 1; i++)
							{
								pf.read(buffer + 1, 1);
								channelBytesLeft--;

								layerBytes[imageLayersRead][(texturePixelBuffer + pixelsRead / layerRects[layerNum].w +
									+(layerRects[layerNum].w - 1 + texturePixelBuffer - pixelsRead % layerRects[layerNum].w) * (layerRects[layerNum].h + texturePixelBuffer * 2))
									* 4 + channelOffset] = buffer[1];
								pixelsRead++;
							}
						}
					}

					//skip over the compression bits, why does it have it for each channel except for the last one? idk
					if (channel != 3)
						pf.seekg(2, std::ios::cur);

					channelBytesLeft = 0;
					pixelsRead = 0;
				}
			}
			else
			{
				//channel order is ARGB in psd, RGBA in png
				for (int channel = 0; channel < 4; channel++)
				{
					if (channel == 0)
						channelOffset = 3;
					else
						channelOffset = channel - 1;

					//read each row's byte count
					for (int i = 0; i < layerRects[layerNum].h; i++)
					{
						pf.read(buffer, 2);
						channelBytesLeft += ((buffer[0] & 0xFF) << 8) | (buffer[1] & 0xFF);
					}

					while (channelBytesLeft > 1)
					{
						//read the count
						pf.read(buffer, 1);
						channelBytesLeft--;

						//RLE compressed part
						if ((int)buffer[0] < 0)
						{
							pf.read(buffer + 1, 1);
							channelBytesLeft--;
							for (int i = 0; i < -buffer[0] + 1; i++)
							{
								layerBytes[imageLayersRead][(texturePixelBuffer + pixelsRead % layerRects[layerNum].w +
									+(texturePixelBuffer + pixelsRead / layerRects[layerNum].w) * (layerRects[layerNum].w + texturePixelBuffer * 2))
									* 4 + channelOffset] = buffer[1];
								pixelsRead++;
							}
						}
						//not RLE compressed part
						else
						{
							for (int i = 0; i < buffer[0] + 1; i++)
							{
								pf.read(buffer + 1, 1);
								channelBytesLeft--;

								layerBytes[imageLayersRead][(texturePixelBuffer + pixelsRead % layerRects[layerNum].w +
									+(texturePixelBuffer + pixelsRead / layerRects[layerNum].w) * (layerRects[layerNum].w + texturePixelBuffer * 2))
									* 4 + channelOffset] = buffer[1];
								pixelsRead++;
							}
						}
					}

					//skip over the compression bits, why does it have it for each channel except for the last one? idk
					if (channel != 3)
						pf.seekg(2, std::ios::cur);

					channelBytesLeft = 0;
					pixelsRead = 0;
				}
			}
			model->meshList[imageLayersRead].name = layerRects[layerNum].layerName;
			model->meshList[imageLayersRead].setPos(layerRects[layerNum].x + (layerRects[layerNum].w - psdWidth) / 2.0f, -layerRects[layerNum].y + (-layerRects[layerNum].h + psdHeight) / 2.0f);
			model->meshList[imageLayersRead].createBasicMesh(rectangles[imageLayersRead].y, rectangles[imageLayersRead].x, rectangles[imageLayersRead].w, rectangles[imageLayersRead].h, rectangles[imageLayersRead].flipped, atlasWidth, atlasHeight);

			//assign a pool of tasks to threads
			texPtr = &layerBytes[imageLayersRead][0];
			if (rectangles[imageLayersRead].flipped)
			{
				results.emplace_back(
					pool.enqueue([texPtr, imageLayersRead, layerRects, layerNum, texturePixelBuffer]
						{
							AlphaBleeding::bleedPng(texPtr, layerRects[layerNum].h + texturePixelBuffer * 2, layerRects[layerNum].w + texturePixelBuffer * 2, texturePixelBuffer);
							return true;
						})
				);
			}
			else
			{
				results.emplace_back(
					pool.enqueue([texPtr, imageLayersRead, layerRects, layerNum, texturePixelBuffer]
						{
							AlphaBleeding::bleedPng(texPtr, layerRects[layerNum].w + texturePixelBuffer * 2, layerRects[layerNum].h + texturePixelBuffer * 2, texturePixelBuffer);
							return true;
						})
				);
			}

			imageLayersRead++;
			break;
		default:
			std::cout << "Compression method not supported: " << compression << std::endl;
			return;
		}
	}

#pragma endregion

	std::cout << "***End of psd file***\n" << std::endl;
	pf.close();

	pool.wait_until_empty();
	pool.wait_until_nothing_in_flight();

	int imageLayer = 0;

	//shove all layer textures into a single texture atlas
	std::cout << "Generating texture atlas... ";;
	//for each texture
	for (int i = 0; i < layerRects.size(); i++)
	{
		if (layerRects[i].layerType == LayerRect::LayerType::image)
		{
			if (!rectangles[imageLayer].flipped)
			{
				//for each row
				for (int row = 0; row < layerRects[i].h + texturePixelBuffer * 2; row++)
				{
					memcpy(&atlasBytes[0]
						+ (rectangles[imageLayer].x
							+ (rectangles[imageLayer].y + row) * atlasWidth)
						* 4,
						&layerBytes[imageLayer][row * (layerRects[i].w + texturePixelBuffer * 2) * 4], (layerRects[i].w + texturePixelBuffer * 2) * 4);
				}
			}
			else
			{
				//for each row
				for (int row = 0; row < layerRects[i].w + texturePixelBuffer * 2; row++)
				{
					memcpy(&atlasBytes[0]
						+ (rectangles[imageLayer].x
							+ (rectangles[imageLayer].y + row) * atlasWidth)
						* 4,
						&layerBytes[imageLayer][row * (layerRects[i].h + texturePixelBuffer * 2) * 4], (layerRects[i].h + texturePixelBuffer * 2) * 4);
				}
			}
			imageLayer++;
		}
	}
	std::cout << "Done" << std::endl;

	//remove alpha, testing purposes
	//for (size_t i = 3; i < atlasBytes.size(); i += 4)
	//	atlasBytes[i] = 255;

	//create texture atlas
	std::cout << "Creating atlas " << atlasWidth << "x" << atlasHeight << "...";;
	stbi_write_png("saves/testExports/textureAtlas.png", atlasWidth, atlasHeight, 4, &(atlasBytes[0]), atlasWidth * 4);
	std::cout << "Done" << std::endl;
}

std::vector<rect_type> TextureLoader::prepareTextureAtlas(std::vector<LayerRect>& layerRects, int texturePixelBuffer, int* atlasWidth, int* atlasHeight)
{
	auto report_successful = [](rect_type&)
	{
		return rectpack2D::callback_result::CONTINUE_PACKING;
	};
	auto report_unsuccessful = [](rect_type&)
	{
		return rectpack2D::callback_result::ABORT_PACKING;
	};

	const auto max_side = 16384;
	const auto discard_step = -4;
	//change to enabled after testing
	const auto runtime_flipping_mode = rectpack2D::flipping_option::ENABLED;

	std::vector<rect_type> rectangles;

	for (int i = 0; i < layerRects.size(); i++)
	{
		if (layerRects[i].layerType == LayerRect::LayerType::image)
			rectangles.emplace_back(rectpack2D::rect_xywhf(0, 0, layerRects[i].w + texturePixelBuffer * 2, layerRects[i].h + texturePixelBuffer * 2, false));
	}

	using rect_ptr = rect_type*;

	auto my_custom_order = [](const rect_ptr a, const rect_ptr b)
	{
		return a->get_wh().pathological_mult() > b->get_wh().pathological_mult();
	};

	const auto result_size = rectpack2D::find_best_packing<spaces_type>(rectangles,
		rectpack2D::make_finder_input(max_side, discard_step, report_successful, report_unsuccessful, runtime_flipping_mode),
		my_custom_order);

	*atlasWidth = nextPower2(result_size.w);
	*atlasHeight = nextPower2(result_size.h);

	return rectangles;
}

//gave up, delete later
//based off of https://learnopengl.com/Advanced-OpenGL/Framebuffers
void TextureLoader::bleedPng(unsigned char* data, int width, int height)
{
	//fbo stuff
	unsigned int fbo;
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "FRAMEBUFFER NOT COMPLETE" << std::endl;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//texture stuff
	unsigned int texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

	//framebuffer stuff
	unsigned int framebuffer;
	glGenFramebuffers(1, &framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

	//color attachment
	unsigned int texColorBuffer;
	glGenTextures(1, &texColorBuffer);
	glBindTexture(GL_TEXTURE_2D, texColorBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texColorBuffer, 0);

	//renderbuffer stuff
	unsigned int rbo;
	glGenRenderbuffers(1, &rbo);

	glBindRenderbuffer(GL_RENDERBUFFER, rbo);

	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);


	glDeleteFramebuffers(1, &fbo);
}

int TextureLoader::nextPower2(int num)
{
	num--;

	num |= num >> 1;
	num |= num >> 2;
	num |= num >> 4;
	num |= num >> 8;
	num |= num >> 16;

	return ++num;
}
