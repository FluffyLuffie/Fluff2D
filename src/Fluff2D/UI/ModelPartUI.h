#pragma once

#include <iostream>
#include <string>
#include <vector>

class ModelPartUI
{
public:
	enum class PartType : int { image = 0, divider = 1, root = 2};

	ModelPartUI() {}
	ModelPartUI(PartType _type);
	~ModelPartUI() {}

	std::string name = "";
	PartType type = PartType::root;

	ModelPartUI *parent;
	std::vector<ModelPartUI> children;

	void printList(int folderCount);
};

