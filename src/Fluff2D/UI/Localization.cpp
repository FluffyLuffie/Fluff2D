#include "Localization.h"


const char* Localization::getTranslation(const char* key)
{
	return translations[key];
}

void Localization::loadLanguage(Language language)
{
	//I can't figure out how languages work so have some very questionable methods
    //If anyone knows how to read txt files into here instead of hard coding, pls help me
	//tried using json parsers, wcstombs_S, widechartomultibyte, everything

	/*
	translations.clear();

	switch (language)
	{
		case Language::English:
			translations["File"] = u8"File";

			translations["General"] = u8"General";

			translations["test"] = u8"english";
			break;
		case Language::Japanese:
			translations["File"] = u8"ファイル";

			translations["General"] = u8"基本";

			translations["test"] = u8"日本語";
			break;
		default:
			break;
	}
	*/
}