#include "TextureLoader.h"

void TextureLoader::loadTexture(unsigned int* texture, const char* fileName, int* width, int* height, int* nrChannels)
{
	glGenTextures(1, texture);
	glBindTexture(GL_TEXTURE_2D, *texture);

	float borderColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
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
void TextureLoader::loadPsdFile(const char* fileName)
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

	//too lazy, assume rest of the header is correct too, just skip over it
	pf.seekg(20, std::ios::cur);
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

#pragma region Layer and Mask Information Section
	//If anything goes wrong, 99% sure it's from this section, especially the Channel Image Data Section

	//layer info
	//skip junk data
	pf.seekg(8, std::ios::cur);

	//layers count as 1, folders count as 2, don't ask me why cause that's how psd works
	pf.read(buffer, 2);
	int layerCount = std::abs(((buffer[0] & 0xFF) << 8) | (buffer[1] & 0xFF));
	std::cout << "Layer count: " << layerCount << std::endl << std::endl;
	layerRects.resize(layerCount);

	//read in layer data
	for (int layerNum = 0; layerNum < layerCount; layerNum++)
	{
		std::cout << "Reading layer " << layerNum << std::endl;

		pf.read(buffer, 16);
		layerRects[layerNum].top = (buffer[0] << 24) | ((buffer[1] & 0xFF) << 16) | ((buffer[2] & 0xFF) << 8) | (buffer[3] & 0xFF);
		layerRects[layerNum].left = (buffer[4] << 24) | ((buffer[5] & 0xFF) << 16) | ((buffer[6] & 0xFF) << 8) | (buffer[7] & 0xFF);
		layerRects[layerNum].bottom = (buffer[8] << 24) | ((buffer[9] & 0xFF) << 16) | ((buffer[10] & 0xFF) << 8) | (buffer[11] & 0xFF);
		layerRects[layerNum].right = (buffer[12] << 24) | ((buffer[13] & 0xFF) << 16) | ((buffer[14] & 0xFF) << 8) | (buffer[15] & 0xFF);
		layerRects[layerNum].calculateDimensions();

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
					//std::cout << "psd layer names suck" << std::endl;
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

		std::cout << "Layer name: " << layerRects[layerNum].layerName << std::endl;

		//folder dividers are named like this idk
		if (layerRects[layerNum].layerName.compare("</Layer set>") == 0)
		{
			std::cout << "Reading a section divider" << std::endl;
			layerRects[layerNum].layerType = LayerType::divider;

			//key is "lsct", I hope
			//also skip a bunch of extra stuff, number of bytes are wrong on documentation but who cares
			pf.seekg(48, std::ios::cur);

			//skip unicode name
			pf.read(buffer, 4);
			pf.seekg(2 * ((buffer[0] << 24) | ((buffer[1] & 0xFF) << 16) | ((buffer[2] & 0xFF) << 8) | (buffer[3] & 0xFF)), std::ios::cur);

			//some junk I think, hope nothing breaks
			pf.seekg(16, std::ios::cur);

			std::cout << std::endl;
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
				std::cout << "Reading a folder" << std::endl;
				layerRects[layerNum].layerType = LayerType::folder;

				//skip a few things
				pf.seekg(44, std::ios::cur);

				//skip unicode name
				pf.read(buffer, 4);
				pf.seekg(2 * ((buffer[0] << 24) | ((buffer[1] & 0xFF) << 16) | ((buffer[2] & 0xFF) << 8) | (buffer[3] & 0xFF)), std::ios::cur);

				//junk
				pf.seekg(16, std::ios::cur);

				std::cout << std::endl;
			}
			//if image layer
			else
			{
				std::cout << "Reading an image" << std::endl;

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

				std::cout << std::endl;
			}
		}
	}

	//Channel Image Data
	std::cout << "Channel Image Data" << std::endl;

	//get rects for each layer
	for (int layerNum = 0; layerNum < layerRects.size(); layerNum++)
	{
		std::cout << "Reading layer " << layerNum << std::endl;

		//figure out compression method
		pf.read(buffer, 2);
		int compression = ((buffer[0] & 0xFF) << 8) | (buffer[1] & 0xFF);

		//get stuff ready to convert to a png
		int imageWidth = layerRects[layerNum].right - layerRects[layerNum].left;
		int imageHeight = layerRects[layerNum].bottom - layerRects[layerNum].top;
		//std::vector <char> imageBytes;
		int channelBytesLeft = 0, channelOffset = 0, pixelsRead = 0, bytesRead = 0;

		switch (compression)
		{
		//Raw Compression
		case 0:
			//I'm guessing this only happens in folders so if not... good luck
			pf.seekg(6, std::ios::cur);

			break;
		//RLE Compression
		case 1:
			layerRects[layerNum].imageBytes.resize(4 * (imageWidth * imageHeight));

			//channel order is ARGB in psd, RGBA in png
			for (int channel = 0; channel < 4; channel++)
			{
				if (channel == 0)
					channelOffset = 3;
				else
					channelOffset = channel - 1;

				//read each row's byte count
				for (int i = 0; i < layerRects[layerNum].bottom - layerRects[layerNum].top; i++)
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
							layerRects[layerNum].imageBytes[4 * pixelsRead + channelOffset] = buffer[1];
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

							layerRects[layerNum].imageBytes[4 * pixelsRead + channelOffset] = buffer[1];
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
			break;
		default:
			std::cout << "Compression method not supported: " << compression << std::endl;

			//Debugging stuff, not meant to actually do stuff
			pf.seekg(-10, std::ios::cur);
			for (int i = 0; i < 80; i++)
			{
				pf.read(buffer, 1);
				std::cout << i << ": " << (int)buffer[0] << std::endl;
			}

			return;
		}
	}

#pragma endregion

	std::cout << "***End of psd file***\n" << std::endl;
	pf.close();

	//create texture atlas
	createTextureAtlas(layerRects);

	//Convert psd to a model

}

void TextureLoader::createTextureAtlas(std::vector<LayerRect>& layerRects)
{
	//testing to make individual png's, change to texture atlas
	for (int i = 0; i < layerRects.size(); i++)
	{
		if (layerRects[i].layerType == LayerType::image)
		{
			std::cout << "Creating png... ";
			stbi_write_png((std::string("saves/testExports/test") + std::to_string(i) + std::string(".png")).c_str(), layerRects[i].width, layerRects[i].height, 4, &(layerRects[i].imageBytes[0]), layerRects[i].width * 4);
			std::cout << "Done" << std::endl;
		}
	}
}
