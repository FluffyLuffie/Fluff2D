#include "ModelMesh.h"

ModelMesh::ModelMesh()
{
	glGenVertexArrays(1, (GLuint*)(&vao));
	glGenBuffers(1, (GLuint*)(&vbo));
	glGenBuffers(1, (GLuint*)(&ebo));

	glBindVertexArray(vao);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
}

ModelMesh::~ModelMesh()
{
	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vbo);
	glDeleteBuffers(1, &ebo);
}

void ModelMesh::render()
{
	glBindVertexArray(vao);
	glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);
}

void ModelMesh::renderInspector()
{
	ImGui::Text("flipped: %s", flipped ? "true" : "false");
	ImGui::Text(name.c_str());
	bool dataChanged = false;

	dataChanged |= ImGui::DragInt("Render Order", &renderOrder);
	ImGui::Separator();

	ImGui::Checkbox("Visible", &visible);
	ImGui::Separator();

	dataChanged |= ImGui::DragFloat2("Position", (float*)&pos);
	dataChanged |= ImGui::DragFloat("Rotation", &rotation);
	dataChanged |= ImGui::DragFloat2("Scale", &scale.x, 0.01f);
	ImGui::Separator();
	dataChanged |= ImGui::ColorEdit4("Mesh Color", &color[0]);
	ImGui::Separator();

	if (dataChanged)
	{
		if (keyformIndex != -1)
		{
			keyforms[keyformIndex].deltaPosition = pos - basePos;
			keyforms[keyformIndex].deltaRotation = rotation - baseRotation;
			keyforms[keyformIndex].deltaScale = scale - baseScale;
			keyforms[keyformIndex].deltaRenderOrder = renderOrder - baseRenderOrder;
			keyforms[keyformIndex].deltaColor = color - baseColor;
		}
		else if (!keyforms.size())
		{
			basePos = pos;
			baseRotation = rotation;
			baseScale = scale;
			baseRenderOrder = renderOrder;
			baseColor = color;
		}
	}

	ImGui::Combo("Blend Mode", &blendMode, blendModes, IM_ARRAYSIZE(blendModes));
}

void ModelMesh::addMeshVertex(glm::vec2 vPos, int atlasWidth, int atlasHeight)
{
	if (flipped)
		addVertex(vPos.x, vPos.y, (atlasPositionX + textureWidth / 2.0f - vPos.y) / atlasWidth, (atlasHeight - atlasPositionY - textureHeight / 2.0f + vPos.x) / atlasHeight);
	else
		addVertex(vPos.x, vPos.y, (atlasPositionX + textureWidth / 2.0f + vPos.x) / atlasWidth, (atlasHeight - atlasPositionY - textureHeight / 2.0f + vPos.y) / atlasHeight);

	vertices[vertices.size() - 1].position = transform * glm::vec4(vertices[vertices.size() - 1].position, 0.0f, 1.0f);
}

void ModelMesh::createBasicMesh(int layerX, int layerY, int layerW, int layerH, bool flip, int atlasWidth, int atlasHeight)
{
	atlasPositionX = layerX;
	atlasPositionY = layerY;
	textureWidth = layerW;
	textureHeight = layerH;
	flipped = flip;

	clearMeshData();

	vertices.reserve(5);
	localVertexPositions.reserve(5);
	originalVertexPositions.reserve(5);

	//create default vertices
	//order is center, top left, top right, bottom right, bottom left (basically clockwise)
	if (flip)
	{
		addMeshVertex(glm::vec2(), atlasWidth, atlasHeight);
		addMeshVertex(glm::vec2(layerH / -2.0f, layerW / 2.0f), atlasWidth, atlasHeight);
		addMeshVertex(glm::vec2(layerH / 2.0f, layerW / 2.0f), atlasWidth, atlasHeight);
		addMeshVertex(glm::vec2(layerH / 2.0f, layerW / -2.0f), atlasWidth, atlasHeight);
		addMeshVertex(glm::vec2(layerH / -2.0f, layerW / -2.0f), atlasWidth, atlasHeight);
	}
	else
	{
		addMeshVertex(glm::vec2(), atlasWidth, atlasHeight);
		addMeshVertex(glm::vec2(layerW / -2.0f, layerH / 2.0f), atlasWidth, atlasHeight);
		addMeshVertex(glm::vec2(layerW / 2.0f, layerH / 2.0f), atlasWidth, atlasHeight);
		addMeshVertex(glm::vec2(layerW / 2.0f, layerH / -2.0f), atlasWidth, atlasHeight);
		addMeshVertex(glm::vec2(layerW / -2.0f, layerH / -2.0f), atlasWidth, atlasHeight);
	}

	//make indices
	indices.reserve(12);
	indices.push_back(0);
	indices.push_back(1);
	indices.push_back(2);
	indices.push_back(0);
	indices.push_back(2);
	indices.push_back(3);
	indices.push_back(0);
	indices.push_back(3);
	indices.push_back(4);
	indices.push_back(0);
	indices.push_back(4);
	indices.push_back(1);
}

void ModelMesh::createTriMesh(int boxCountX, int boxCountY, int atlasWidth, int atlasHeight)
{
	clearMeshData();

	//create vertices for boxes
	vertices.reserve((boxCountX + 1) * (boxCountY + 1) + boxCountX * boxCountY);
	localVertexPositions.reserve((boxCountX + 1) * (boxCountY + 1) + boxCountX * boxCountY);
	originalVertexPositions.reserve((boxCountX + 1) * (boxCountY + 1) + boxCountX * boxCountY);

	if (flipped)
	{
		//normal box
		for (int y = 0; y <= boxCountY; y++)
			for (int x = 0; x <= boxCountX; x++)
				addMeshVertex(glm::vec2(textureHeight * (-0.5f + 1.0f / boxCountX * x), textureWidth * (-0.5f + 1.0f / boxCountY * y)), atlasWidth, atlasHeight);

		//put dot in middle
		for (int y = 0; y < boxCountY; y++)
			for (int x = 0; x < boxCountX; x++)
				addMeshVertex(glm::vec2(textureHeight * (-0.5f + 1.0f / boxCountX * (x + 0.5f)), textureWidth * (-0.5f + 1.0f / boxCountY * (y + 0.5f))), atlasWidth, atlasHeight);
	}
	else
	{
		//normal box
		for (int y = 0; y <= boxCountY; y++)
			for (int x = 0; x <= boxCountX; x++)
				addMeshVertex(glm::vec2(textureWidth * (-0.5f + 1.0f / boxCountX * x), textureHeight * (-0.5f + 1.0f / boxCountY * y)), atlasWidth, atlasHeight);

		//put dot in middle
		for (int y = 0; y < boxCountY; y++)
			for (int x = 0; x < boxCountX; x++)
				addMeshVertex(glm::vec2(textureWidth * (-0.5f + 1.0f / boxCountX * (x + 0.5f)), textureHeight * (-0.5f + 1.0f / boxCountY * (y + 0.5f))), atlasWidth, atlasHeight);
	}

	Triangulator::triangulate(vertices, indices);
}

void ModelMesh::startMeshEdit()
{
	transform = glm::translate(glm::mat4(1.0f), glm::vec3(originalPos, 0.0f));
	for (int i = 0; i < localVertexPositions.size(); i++)
		vertices[i].position = transform * glm::vec4(originalVertexPositions[i], 0.0f, 1.0f);
}

void ModelMesh::removeVertex(int index)
{
	vertices.erase(vertices.begin() + index);
	localVertexPositions.erase(localVertexPositions.begin() + index);
	originalVertexPositions.erase(originalVertexPositions.begin() + index);

	for (int i = 0; i < keyforms.size(); i++)
	{
		keyforms[i].deltaVertices.erase(index);

		//shift the position of keyforms
		std::unordered_map<int, glm::vec2> temp;
		for (auto const &[i, v] : keyforms[i].deltaVertices)
		{
			if (i > index)
				temp[i - 1] = v;
			else
				temp[i] = v;
		}
		keyforms[i].deltaVertices = temp;
	}

	Triangulator::triangulate(vertices, indices);
}

//TODO: find a better way for reducing the number of vertices
void ModelMesh::autoMesh(std::filesystem::path directoryPath, int atlasWidth, int atlasHeight, int edgeOut, int edgeIn, int edgeSpacing, int insideSpacing, unsigned char threshold)
{
	std::ifstream textureFile = std::ifstream(directoryPath / (("f2d_") + std::to_string(textureIndex) + ".tmp"), std::ios::binary);
	if (!textureFile.is_open())
		Log::error("Temporary texture path not found");

	int offsets[][2] = { { 0, -1}, {-1,  0}, { 1,  0}, { 0,  1} };

	const int width = textureWidth + 2 * edgeOut;
	const int height = textureHeight + 2 * edgeOut;
	const int N = width * height;

	std::vector<unsigned char> texBytes(N * 4);

	textureFile.read((char*)&texBytes[0], N * 4);
	textureFile.close();

	std::vector<glm::vec2> tempVertices, finalVertices;

	std::vector<unsigned char> originalAlpha;
	originalAlpha.resize(N);
	for (int y = 0; y < textureHeight; y++)
		for (int x = 0; x < textureWidth; x++)
			originalAlpha[x + edgeOut + width * (height - edgeOut - y - 1)] = texBytes[(x + y * textureWidth) * 4 + 3];

	std::vector<unsigned char> filled = originalAlpha;
	std::vector<unsigned char> queued = originalAlpha;

	std::vector<int> pending;
	std::vector<int> pendingNext;

	pending.reserve(N);
	pendingNext.reserve(N);

	//outer edge
	for (int y = edgeOut - 1; y < height - edgeOut + 1; y++)
		for (int x = edgeOut - 1; x < width - edgeOut + 1; x++)
			if (originalAlpha[x + y * width] <= threshold)
			{
				for (int i = 0; i < 4; i++)
				{
					int adjX = x + offsets[i][0], adjY = y + offsets[i][1];
					if (originalAlpha[adjX + adjY * width] > threshold)
					{
						pending.push_back(x + y * width);
						break;
					}
				}
			}
	for (int loop = 0; loop < edgeOut - 1; loop++)
	{
		pendingNext.clear();

		for (int i = 0; i < pending.size(); i++)
		{
			filled[pending[i]] = 255;
			int x = pending[i] % width;
			int y = pending[i] / width;

			for (int j = 0; j < 4; j++)
			{
				int adjX = x + offsets[j][0], adjY = y + offsets[j][1];
				if (queued[adjX + adjY * width] <= threshold)
				{
					queued[adjX + adjY * width] = 255;
					pendingNext.push_back(adjX + adjY * width);
				}
			}
		}
		pending.swap(pendingNext);
	}

	for (int i = 0; i < pending.size(); i++)
		if (flipped)
			tempVertices.push_back(glm::vec2(pending[i] / width - textureHeight / 2 - edgeOut, width - 1 - pending[i] % width - textureWidth / 2 - edgeOut));
		else
			tempVertices.push_back(glm::vec2(pending[i] % width - textureWidth / 2 - edgeOut, pending[i] / width - textureHeight / 2 - edgeOut));

	std::sort(tempVertices.begin(), tempVertices.end(), compareAngle);
	for (int i = 0; i < tempVertices.size(); i++)
		if (i % edgeSpacing == 0)
			finalVertices.push_back(tempVertices[i]);
	tempVertices.clear();

	//inner edge
	filled = originalAlpha;
	queued = originalAlpha;
	pending.clear();

	for (int y = edgeOut - 1; y < height - edgeOut + 1; y++)
		for (int x = edgeOut - 1; x < width - edgeOut + 1; x++)
			if (originalAlpha[x + y * width] > threshold)
			{
				for (int i = 0; i < 4; i++)
				{
					int adjX = x + offsets[i][0], adjY = y + offsets[i][1];
					if (originalAlpha[adjX + adjY * width] <= threshold)
					{
						pending.push_back(x + y * width);
						queued[x + y * width] = 0;
						break;
					}
				}
			}

	for (int loop = 0; loop < edgeIn - 1; loop++)
	{
		pendingNext.clear();

		for (int i = 0; i < pending.size(); i++)
		{
			filled[pending[i]] = 0;
			queued[pending[i]] = 0;
			int x = pending[i] % width;
			int y = pending[i] / width;

			for (int j = 0; j < 4; j++)
			{
				int adjX = x + offsets[j][0], adjY = y + offsets[j][1];
				if (queued[adjX + adjY * width] > threshold)
				{
					queued[adjX + adjY * width] = 0;
					pendingNext.push_back(adjX + adjY * width);
				}
			}
		}
		pending.swap(pendingNext);
	}

	for (int i = 0; i < pending.size(); i++)
		if (flipped)
			tempVertices.push_back(glm::vec2(pending[i] / width - textureHeight / 2 - edgeOut, width - 1 - pending[i] % width - textureWidth / 2 - edgeOut));
		else
			tempVertices.push_back(glm::vec2(pending[i] % width - textureWidth / 2 - edgeOut, pending[i] / width - textureHeight / 2 - edgeOut));

	std::sort(tempVertices.begin(), tempVertices.end(), compareAngle);
	for (int i = 0; i < tempVertices.size(); i++)
		if (i % edgeSpacing == 0)
			finalVertices.push_back(tempVertices[i]);
	tempVertices.clear();

	//inside
	while (pending.size())
	{
		for (int loop = 0; loop < insideSpacing - 1; loop++)
		{
			pendingNext.clear();

			for (int i = 0; i < pending.size(); i++)
			{
				filled[pending[i]] = 0;
				queued[pending[i]] = 0;
				int x = pending[i] % width;
				int y = pending[i] / width;

				for (int j = 0; j < 4; j++)
				{
					int adjX = x + offsets[j][0], adjY = y + offsets[j][1];
					if (queued[adjX + adjY * width] > threshold)
					{
						queued[adjX + adjY * width] = 0;
						pendingNext.push_back(adjX + adjY * width);
					}
				}
			}
			pending.swap(pendingNext);
		}

		for (int i = 0; i < pending.size(); i++)
			if (flipped)
				tempVertices.push_back(glm::vec2(pending[i] / width - textureHeight / 2 - edgeOut, width - 1 - pending[i] % width - textureWidth / 2 - edgeOut));
			else
				tempVertices.push_back(glm::vec2(pending[i] % width - textureWidth / 2 - edgeOut, pending[i] / width - textureHeight / 2 - edgeOut));

		std::sort(tempVertices.begin(), tempVertices.end(), compareAngle);
		for (int i = 0; i < tempVertices.size(); i++)
			if (i % insideSpacing == 0)
				finalVertices.push_back(tempVertices[i]);
		tempVertices.clear();
	}

	if (finalVertices.size() > 2)
	{
		clearMeshData();
		for (int i = 0; i < finalVertices.size(); i++)
			addMeshVertex(finalVertices[i], atlasWidth, atlasHeight);
		Triangulator::triangulate(vertices, indices);
	}
	else
		Log::error("Auto Mesh Failed");
}

glm::vec2 ModelMesh::posToTexCoord(const glm::vec2& vPos, int atlasWidth, int atlasHeight)
{
	if (flipped)
		return glm::vec2((atlasPositionX + textureWidth / 2.0f - vPos.y) / atlasWidth, (atlasHeight - atlasPositionY - textureHeight / 2.0f + vPos.x) / atlasHeight);
	else
		return glm::vec2((atlasPositionX + textureWidth / 2.0f + vPos.x) / atlasWidth, (atlasHeight - atlasPositionY - textureHeight / 2.0f + vPos.y) / atlasHeight);
}
