#include "SaveSystem.h"

void SaveSystem::saveModel(std::shared_ptr<Model> model, int version)
{
	Log::info("Saving model");

	std::ofstream output("saves/testExports/fluffModel.ftd", std::ios::binary);
	if (!output.is_open())
	{
		Log::error("Failed to initialize output save file");
		return;
	}

	//file signature
	output << "ftd8";

	//version
	output << version;

	//save parts structure
	output << static_cast<int>(model->layerStructure.size());
	for (int i = 0; i < model->layerStructure.size(); i++)
	{
		//partsOutput(&output, model->layerStructure[i]);
	}

	//save parameter stuff
	/*
	output << static_cast<int>(model->paramMap.size());
	for (const auto& [key, value] : model->paramMap)
	{
		output << key << value.defaultValue << value.maxValue << value.minValue;
	}
	*/

	//save mesh data
	output << static_cast<int>(model->modelMeshes.size());
	for (int i = 0; i < model->modelMeshes.size(); i++)
	{
		output << model->modelMeshes[i]->pos.x;
		output << model->modelMeshes[i]->pos.y;
		output << static_cast<int>(model->modelMeshes[i]->vertices.size());
		for (int j = 0; j < model->modelMeshes[i]->vertices.size(); j++)
		{
			output << model->modelMeshes[i]->vertices[j].position.x;
			output << model->modelMeshes[i]->vertices[j].position.y;
			output << model->modelMeshes[i]->vertices[j].position.x;
			output << model->modelMeshes[i]->vertices[j].position.y;
		}
	}

	output.close();
}

void SaveSystem::loadSettings()
{
	//add reading from file

	Localization::loadLanguage(Settings::selectedLanguage);
}

void SaveSystem::saveSettings()
{
}

void SaveSystem::partsOutput(std::ofstream* output, std::shared_ptr<ModelPartUI> modelPart)
{
	*output << static_cast<char>(modelPart->type);
	*output << modelPart->name.c_str();

	if (modelPart->type == ModelPartUI::PartType::divider)
	{
		*output << static_cast<int>(modelPart->children.size());
		for (int i = 0; i < modelPart->children.size(); i++)
			partsOutput(output, modelPart->children[i]);
	}
}
