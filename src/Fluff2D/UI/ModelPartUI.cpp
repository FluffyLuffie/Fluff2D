#include "ModelPartUI.h"

ModelPartUI::ModelPartUI(PartType _type)
{
	type = _type;
}

ModelPartUI::ModelPartUI(PartType _type, const std::string& partName)
{
	type = _type;
	name = partName;
}
