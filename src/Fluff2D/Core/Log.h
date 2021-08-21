#pragma once

//#include "../UI/Localization.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

class Log
{
public:
    inline static ImGuiTextBuffer buf;
    inline static ImGuiTextFilter filter;
    inline static ImVector<int> lineOffsets;

    static void clear();
    static void addLog(const char* fmt, ...) IM_FMTARGS(2);
    static void draw(const char* title);

    static void logInfo(const char* fmt, ...) IM_FMTARGS(2);
    static void logWarning(const char* fmt, ...) IM_FMTARGS(2);
    static void logError(const char* fmt, ...) IM_FMTARGS(2);
};

