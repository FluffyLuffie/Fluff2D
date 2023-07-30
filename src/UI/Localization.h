#pragma once

#include <iostream>
#include <string>
#include <unordered_map>

class Localization
{
public:
	enum class Language : int {
		English = 0,
		Japanese = 1};

	static const char* getTranslation(const char* key);
	static void loadLanguage(Language language);

private:
	inline static std::unordered_map<std::string, const char*> translations;
};

