#include "ModelPartUI.h"

ModelPartUI::ModelPartUI(PartType _type)
{
	type = _type;
}

void ModelPartUI::printList(int folderCount)
{
	switch (type)
	{
	case PartType::image:
		std::cout << std::string(folderCount * 4, ' ') << name << std::endl;
		break;
	case PartType::divider:
		std::cout << std::string(folderCount * 4, ' ') << name << std::endl;

		for (int i = children.size() - 1; i >= 0; i--)
		{
			children[i].printList(folderCount + 1);
		}
		break;
	default:
		for (int i = children.size() - 1; i >= 0; i--)
		{
			children[i].printList(folderCount + 1);
		}
		break;
	}
}
