#include "TextureLoader.h"

void TextureLoader::loadTexture(unsigned int* texture, const char* fileName, int* width, int* height, int* nrChannels)
{
	glGenTextures(1, texture);
	glBindTexture(GL_TEXTURE_2D, *texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	unsigned char *data = stbi_load(fileName, width, height, nrChannels, 0);

	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, *width, *height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
		Log::logError("Failed to load texture");
	stbi_image_free(data);
}

//I only have Clip Studio Paint so idk if it works in Photoshop
//Very high chance of breaking if you so something besides just plain raster layers and folders
//Read this if you want to understand a fraction of the pain I went for you, and others if this becomes an open source project like I intended to
//https://www.adobe.com/devnet-apps/photoshop/fileformatashtml/
bool TextureLoader::loadPsdFile(const char* fileName, std::shared_ptr<Model> model)
{
	//might move this to a class variable later
	std::vector <LayerRect> layerRects;

	Log::logInfo("Opening %s", fileName);

	//check if file successfully opened
	std::ifstream pf (fileName, std::ios::binary);
	if (!pf.is_open())
	{
		Log::logError("Error opening psd file");
		return false;
	}

	char buffer[16];
	pf.seekg(0, std::ios::beg);

#pragma region File Header Section
	//check signature
	pf.read(buffer, 4);
	if (std::strncmp(buffer, "8BPS", 4) != 0)
	{
		Log::logError("PSD file has incorrect signature");
		return false;
	}

	//check version
	pf.read(buffer, 2);
	if (buffer[0] != 0 || buffer[1] != 1)
	{
		Log::logError("PSD file is incorrect version");
		return false;
	}

	//skip over junk
	pf.seekg(8, std::ios::cur);

	//get width and height of psd file
	pf.read(buffer, 4);
	model->psdDimension.y = (buffer[0] << 24) | ((buffer[1] & 0xFF) << 16) | ((buffer[2] & 0xFF) << 8) | (buffer[3] & 0xFF);
	pf.read(buffer, 4);
	model->psdDimension.x = (buffer[0] << 24) | ((buffer[1] & 0xFF) << 16) | ((buffer[2] & 0xFF) << 8) | (buffer[3] & 0xFF);
	model->updateCanvasCoord();

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
	//negative layer counts exists, why does it even do it this way
	pf.read(buffer, 2);
	int layerCount = std::abs(((buffer[0] & 0xFF) << 24) | ((buffer[1] & 0xFF) << 16));
	layerCount = layerCount >> 16;

	layerRects.resize(layerCount);

	std::shared_ptr<ModelPartUI> currentFolder;
	//read in layer data
	for (int layerNum = 0; layerNum < layerCount; layerNum++)
	{
		pf.read(buffer, 16);
		layerRects[layerNum].y = (buffer[0] << 24) | ((buffer[1] & 0xFF) << 16) | ((buffer[2] & 0xFF) << 8) | (buffer[3] & 0xFF);
		layerRects[layerNum].x = (buffer[4] << 24) | ((buffer[5] & 0xFF) << 16) | ((buffer[6] & 0xFF) << 8) | (buffer[7] & 0xFF);
		layerRects[layerNum].h = ((buffer[8] << 24) | ((buffer[9] & 0xFF) << 16) | ((buffer[10] & 0xFF) << 8) | (buffer[11] & 0xFF)) - layerRects[layerNum].y;
		layerRects[layerNum].w = ((buffer[12] << 24) | ((buffer[13] & 0xFF) << 16) | ((buffer[14] & 0xFF) << 8) | (buffer[15] & 0xFF)) - layerRects[layerNum].x;

		//number of channels
		pf.read(buffer, 2);
		int channels = std::abs(((buffer[0] & 0xFF) << 8) | (buffer[1] & 0xFF));

		//skip over a bunch of stuff cause everything is gibberish
		//read if toggle visibility
		pf.seekg(channels * 6 + 10, std::ios::cur);
		pf.read(buffer, 1);
		layerRects[layerNum].visible = !(buffer[0] & 2);
		pf.seekg(5, std::ios::cur);

		//layer mask data, nobody cares about you either
		pf.read(buffer, 4);
		pf.seekg((buffer[0] << 24) | ((buffer[1] & 0xFF) << 16) | ((buffer[2] & 0xFF) << 8) | (buffer[3] & 0xFF), std::ios::cur);

		//layer blending ranges, go away too
		pf.read(buffer, 4);
		pf.seekg((buffer[0] << 24) | ((buffer[1] & 0xFF) << 16) | ((buffer[2] & 0xFF) << 8) | (buffer[3] & 0xFF), std::ios::cur);

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
				else
					pf.seekg(-3, std::ios::cur);
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


		//folder divider names hopefully start with this, Clip Studio Paint is "</Layer set", Krita is "</Layer group>"
		if (layerRects[layerNum].layerName.compare(0, 7, "</Layer") == 0)
		{
			if (!currentFolder)
			{
				model->layerStructure.emplace_back(std::make_shared<ModelPartUI>(ModelPartUI::PartType::divider));
				currentFolder = model->layerStructure.back();
			}
			else
			{
				currentFolder->children.emplace_back(std::make_shared<ModelPartUI>(ModelPartUI::PartType::divider));
				currentFolder->children.back()->parent = currentFolder;
				currentFolder = currentFolder->children.back();
			}

			layerRects[layerNum].layerType = LayerRect::LayerType::divider;

			pf.read(buffer, 4);
			int code = (buffer[0] << 24) | ((buffer[1] & 0xFF) << 16) | ((buffer[2] & 0xFF) << 8) | (buffer[3] & 0xFF);

			switch (code)
			{
				//lsct, divider signature for Clip Studio Paint
				case 1819501428:
					//also skip a bunch of extra stuff, number of bytes are wrong on documentation but who cares
					pf.seekg(44, std::ios::cur);

					//skip unicode name
					pf.read(buffer, 4);
					pf.seekg(2 * ((buffer[0] << 24) | ((buffer[1] & 0xFF) << 16) | ((buffer[2] & 0xFF) << 8) | (buffer[3] & 0xFF)), std::ios::cur);

					//some junk I think, hope nothing breaks
					pf.seekg(16, std::ios::cur);
					break;
				//luni, divider signature for Krita
				case 1819635305:
					//skip unicode name
					pf.read(buffer, 4);
					pf.seekg(((buffer[0] << 24) | ((buffer[1] & 0xFF) << 16) | ((buffer[2] & 0xFF) << 8) | (buffer[3] & 0xFF)), std::ios::cur);

					//should say 8BIM lsct, skip section divider setting
					pf.seekg(24, std::ios::cur);
					break;
			}
			
		}
		//if not divider
		else
		{
			//documentation is horrible why is Additional Layer Information section in a random place

			//distinguish if folder or image layer
			pf.read(buffer, 4);

			int code = (buffer[0] << 24) | ((buffer[1] & 0xFF) << 16) | ((buffer[2] & 0xFF) << 8) | (buffer[3] & 0xFF);
			switch (code)
			{
				//lsct, divider end for Clip Studio Paint
				case 1819501428:
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
					break;

				//lspf, Clip Studio Paint raster layers start wit this
				case 1819504742:
					if (!currentFolder)
					{
						model->layerStructure.emplace_back(std::make_shared<ModelPartUI>(ModelPartUI::PartType::image, layerRects[layerNum].layerName));
					}
					else
					{
						currentFolder->children.emplace_back(std::make_shared<ModelPartUI>(ModelPartUI::PartType::image));
						currentFolder->children.back()->name = layerRects[layerNum].layerName;
					}

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
					break;

				//luni, both Krita raster and divider end starts with this
				case 1819635305:
					//skip unicode name and junk
					//for some reason this is not the number of characters but the number of bytes?
					pf.read(buffer, 4);
					pf.seekg((buffer[0] << 24) | ((buffer[1] & 0xFF) << 16) | ((buffer[2] & 0xFF) << 8) | (buffer[3] & 0xFF), std::ios::cur);

					//check to see if divider end for Krita
					pf.read(buffer, 4);
					//if folder
					if (std::strncmp(buffer, "8BIM", 4) == 0)
					{
						currentFolder->name = layerRects[layerNum].layerName;
						currentFolder = currentFolder->parent;

						layerRects[layerNum].layerType = LayerRect::LayerType::folder;
						pf.seekg(20, std::ios::cur);
					}
					//if raster layer
					else
					{
						if (!currentFolder)
						{
							model->layerStructure.emplace_back(std::make_shared<ModelPartUI>(ModelPartUI::PartType::image, layerRects[layerNum].layerName));
						}
						else
						{
							currentFolder->children.emplace_back(std::make_shared<ModelPartUI>(ModelPartUI::PartType::image));
							currentFolder->children.back()->name = layerRects[layerNum].layerName;
						}
						pf.seekg(-4, std::ios::cur);
					}
					break;
				default:
					Log::logError("Unsupported layer code: %c%c%c%c", buffer[0], buffer[1], buffer[2], buffer[3]);
					return false ;
			}
			
		}
	}

	//TODO: maybe make texture buffer scale according to each rect's size?
	int texturePixelBuffer = 10;
	auto rectangles = prepareTextureAtlas(layerRects, texturePixelBuffer, &model->atlasWidth, &model->atlasHeight);

	std::vector <unsigned char> atlasBytes;
	atlasBytes.resize(model->atlasWidth * model->atlasHeight * 4);

	for (int i = 0; i < rectangles.size(); i++)
	{
		model->modelMeshes.push_back(std::make_shared<ModelMesh>());
		model->modelMeshes.back()->parent = model;
		model->modelMeshes.back()->baseRenderOrder = i * 10;
		model->children.push_back(model->modelMeshes.back());
		model->children.back()->parent = model;
	}

	//Channel Image Data

	//keeps track of how many image layers read
	int imageLayersRead = 0;

	std::vector <std::vector <unsigned char>> layerBytes;
	layerBytes.resize(rectangles.size());
	//idk how many threads to create
	progschj::ThreadPool pool(std::thread::hardware_concurrency() - 1);
	std::vector<std::future<bool>> results;
	unsigned char* texPtr;

	//get rects for each layer
	for (int layerNum = 0; layerNum < layerRects.size(); layerNum++)
	{
		unsigned char* meshAlpha;

		//figure out compression method
		pf.read(buffer, 2);
		int compression = ((buffer[0] & 0xFF) << 8) | (buffer[1] & 0xFF);

		int channelBytesLeft = 0, channelOffset = 0, pixelsRead = 0, texW, texH;

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
			layerBytes[imageLayersRead].resize(layerRects[layerNum].w * layerRects[layerNum].h * 4);
			model->modelMeshes[imageLayersRead]->texAlpha.resize(layerRects[layerNum].w * layerRects[layerNum].h);
			meshAlpha = &model->modelMeshes[imageLayersRead]->texAlpha[0];

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
								layerBytes[imageLayersRead][(pixelsRead / layerRects[layerNum].w
									+ (layerRects[layerNum].w - 1 - pixelsRead % layerRects[layerNum].w) * layerRects[layerNum].h)
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

								layerBytes[imageLayersRead][(pixelsRead / layerRects[layerNum].w
									+ (layerRects[layerNum].w - 1 - pixelsRead % layerRects[layerNum].w) * (layerRects[layerNum].h))
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
								layerBytes[imageLayersRead][(pixelsRead % layerRects[layerNum].w
									+ (pixelsRead / layerRects[layerNum].w) * layerRects[layerNum].w)
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

								layerBytes[imageLayersRead][(pixelsRead % layerRects[layerNum].w
									+ (pixelsRead / layerRects[layerNum].w) * layerRects[layerNum].w)
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

			model->modelMeshes[imageLayersRead]->name = layerRects[layerNum].layerName;
			model->modelMeshes[imageLayersRead]->visible = layerRects[layerNum].visible;
			model->modelMeshes[imageLayersRead]->pos = glm::vec2(layerRects[layerNum].x + (layerRects[layerNum].w - model->psdDimension.x) / 2.0f, -layerRects[layerNum].y + (-layerRects[layerNum].h + model->psdDimension.y) / 2.0f);
			model->modelMeshes[imageLayersRead]->originalPos = model->modelMeshes[imageLayersRead]->pos;
			model->modelMeshes[imageLayersRead]->basePos = model->modelMeshes[imageLayersRead]->pos;
			model->modelMeshes[imageLayersRead]->createBasicMesh(rectangles[imageLayersRead].x, rectangles[imageLayersRead].y, rectangles[imageLayersRead].w, rectangles[imageLayersRead].h, rectangles[imageLayersRead].flipped, model->atlasWidth, model->atlasHeight);

			//assign a pool of tasks to threads
			texPtr = &layerBytes[imageLayersRead][0];
			if (rectangles[imageLayersRead].flipped)
			{
				texW = layerRects[layerNum].h;
				texH = layerRects[layerNum].w;
			}
			else
			{
				texW = layerRects[layerNum].w;
				texH = layerRects[layerNum].h;
			}

			results.emplace_back(
				pool.enqueue([texPtr, meshAlpha, texW, texH]
					{
						premultAlpha(texPtr, meshAlpha, texW, texH);
						return true;
					})
			);

			imageLayersRead++;
			break;
		default:
			Log::logError("Compression method not supported: %d", compression);
			return false;
		}
	}

#pragma endregion

	Log::logInfo("Finished reading PSD");
	pf.close();

	int imageLayer = 0;

	//shove all layer textures into a single texture atlas
	Log::logInfo("Generating texture atlas");
	//for each texture
	for (int i = 0; i < layerRects.size(); i++)
	{
		if (layerRects[i].layerType == LayerRect::LayerType::image)
		{
			if (!rectangles[imageLayer].flipped)
			{
				//for each row
				for (int row = 0; row < layerRects[i].h; row++)
				{
					memcpy(&atlasBytes[0]
						+ (rectangles[imageLayer].x
							+ (model->atlasHeight - rectangles[imageLayer].y - row - 1) * model->atlasWidth)
						* 4,
						&layerBytes[imageLayer][row * layerRects[i].w * 4], layerRects[i].w * 4);
				}
			}
			else
			{
				//for each row
				for (int row = 0; row < layerRects[i].w; row++)
				{
					memcpy(&atlasBytes[0]
						+ (rectangles[imageLayer].x
							+ (model->atlasHeight - rectangles[imageLayer].y - row - 1) * model->atlasWidth)
						* 4,
						&layerBytes[imageLayer][row * layerRects[i].h * 4], layerRects[i].h * 4);
				}
			}
			imageLayer++;
		}
	}

	//remove alpha, testing purposes
	//for (size_t i = 3; i < atlasBytes.size(); i += 4)
	//	atlasBytes[i] = 255;

	//test not having to make png
	glGenTextures(1, &model->textureID);
	glBindTexture(GL_TEXTURE_2D, model->textureID);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, model->atlasWidth, model->atlasHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, &atlasBytes[0]);
	glGenerateMipmap(GL_TEXTURE_2D);

	//writing to png
	//stbi_write_png("saves/testExports/textureAtlas.png", model->atlasWidth, model->atlasHeight, 4, &(atlasBytes[0]), model->atlasWidth * 4);
	//Log::logInfo("Finished generating texture atlas (%d %d)", model->atlasWidth, model->atlasHeight);
	return true;
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
	//turn on after testing
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

	//allign to center
	for (int i = 0; i < rectangles.size(); i++)
	{
		rectangles[i].x += texturePixelBuffer;
		rectangles[i].y += texturePixelBuffer;
		rectangles[i].w -= texturePixelBuffer * 2;
		rectangles[i].h -= texturePixelBuffer * 2;
	}

	return rectangles;
}

void TextureLoader::premultAlpha(unsigned char* image, unsigned char* meshAlpha, int width, int height)
{
	const int N = width * height;

	for (size_t i = 0, j = 3; i < N; i++, j += 4)
	{
		image[j - 1] = (unsigned char)(image[j - 1] * (image[j] / 255.0f));
		image[j - 2] = (unsigned char)(image[j - 2] * (image[j] / 255.0f));
		image[j - 3] = (unsigned char)(image[j - 3] * (image[j] / 255.0f));

		meshAlpha[i] = image[j];
	}
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
