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

    static void info(const char* fmt, ...) IM_FMTARGS(2);
    static void warning(const char* fmt, ...) IM_FMTARGS(2);
    static void error(const char* fmt, ...) IM_FMTARGS(2);
};

