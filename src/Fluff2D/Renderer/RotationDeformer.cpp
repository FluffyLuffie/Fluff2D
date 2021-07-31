#include "RotationDeformer.h"

void RotationDeformer::deformEffect(glm::mat4& transform)
{
	std::cout << "Rotation deformer effect" << std::endl;

	if (parent)
		parent->deformEffect(transform);
}
