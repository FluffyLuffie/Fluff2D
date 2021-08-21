#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

#include "Settings.h"
#include "../UI/Localization.h"
#include "../Renderer/Model.h"
#include "../Renderer/LayerRect.h"

#include "rectpack2D/finders_interface.h"

using spaces_type = rectpack2D::empty_spaces<true, rectpack2D::default_empty_spaces>;
using rect_type = rectpack2D::output_rect_t<spaces_type>;

class SaveSystem
{
public:
	static void saveModel(std::shared_ptr<Model> model, int version);
	static void loadSettings();
	static void saveSettings();

private:
	static void partsOutput(std::ofstream* output, std::shared_ptr<ModelPartUI> modelPart);
};

