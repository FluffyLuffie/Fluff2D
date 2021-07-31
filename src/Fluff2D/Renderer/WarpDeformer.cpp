#include "WarpDeformer.h"

void WarpDeformer::deformEffect(glm::mat4& transform)
{
	std::cout << "Warp deformer effect" << std::endl;

	if (parent)
		parent->deformEffect(transform);
}
