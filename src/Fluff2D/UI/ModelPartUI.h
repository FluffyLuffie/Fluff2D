#pragma once

#include <iostream>
#include <string>
#include <vector>

class ModelPartUI
{
public:
	enum class PartType : char { image = 0, divider = 1, warpDeformer = 2, rotationDeformer = 3};

	ModelPartUI() {}
	ModelPartUI(PartType _type);
	ModelPartUI(PartType _type, const std::string &partName);
	~ModelPartUI() {}

	std::string name = "";
	PartType type = PartType::image;

	std::shared_ptr<ModelPartUI> parent;
	std::vector<std::shared_ptr<ModelPartUI>> children;
};

