#pragma once

#include <vector>

class Parameter
{
public:
	float value = 0.0f, defaultValue = 0.0f, maxValue = 30.0f, minValue = -30.0f;
	std::vector<float> keyValues;

	Parameter() {}
	Parameter(float val) { maxValue = val; minValue = -val; }
	Parameter(float maxVal, float minVal) { maxValue = maxVal; minValue = minVal; }
	Parameter(float defaultVal, float maxVal, float minVal) { value = defaultVal;  defaultValue = defaultVal;  maxValue = maxVal; minValue = minVal; }
	~Parameter() {}
};

