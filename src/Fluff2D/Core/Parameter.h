#pragma once

#include <vector>

class Parameter
{
public:
	float defaultValue = 0.0f, maxValue = 0.0f, minValue = 0.0f;
	std::vector<float> keyValues;

	Parameter() {}
	Parameter(float val) { maxValue = val; minValue = -val; }
	Parameter(float maxVal, float minVal) { maxValue = maxVal; minValue = minVal; }
	Parameter(float defaultVal, float maxVal, float minVal) { defaultValue = defaultVal;  maxValue = maxVal; minValue = minVal; }
	~Parameter() {}

	void snapToClosest();
};

