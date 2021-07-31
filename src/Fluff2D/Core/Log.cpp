#include "Log.h"

void Log::clear()
{
	buf.clear();
	lineOffsets.clear();
	lineOffsets.push_back(0);
}

void Log::addLog(const char* fmt, ...)
{
    int old_size = buf.size();
    va_list args;
    va_start(args, fmt);
    buf.appendfv(fmt, args);
    va_end(args);
    for (int new_size = buf.size(); old_size < new_size; old_size++)
        if (buf[old_size] == '\n')
            lineOffsets.push_back(old_size + 1);
}

void Log::draw(const char* title)
{
    if (!ImGui::Begin(title))
    {
        ImGui::End();
        return;
    }

    // Main window
    bool clearPressed = ImGui::Button("Clear");
    ImGui::SameLine();
    bool copy = ImGui::Button("Copy");
    ImGui::SameLine();
    filter.Draw("Filter", -100.0f);

    ImGui::Separator();
    ImGui::BeginChild("scrolling", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

    if (clearPressed)
        clear();
    if (copy)
        ImGui::LogToClipboard();

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
    const char* bufPtr = buf.begin();
    const char* buf_end = buf.end();
    if (filter.IsActive())
    {
        for (int line_no = 0; line_no < lineOffsets.Size; line_no++)
        {
            const char* line_start = bufPtr + lineOffsets[line_no];
            const char* line_end = (line_no + 1 < lineOffsets.Size) ? (bufPtr + lineOffsets[line_no + 1] - 1) : buf_end;
            if (filter.PassFilter(line_start, line_end))
                ImGui::TextUnformatted(line_start, line_end);
        }
    }
    else
    {
        ImGuiListClipper clipper;
        clipper.Begin(lineOffsets.Size);
        while (clipper.Step())
        {
            for (int line_no = clipper.DisplayStart; line_no < clipper.DisplayEnd; line_no++)
            {
                const char* line_start = bufPtr + lineOffsets[line_no];
                const char* line_end = (line_no + 1 < lineOffsets.Size) ? (bufPtr + lineOffsets[line_no + 1] - 1) : buf_end;
                ImGui::TextUnformatted(line_start, line_end);
            }
        }
        clipper.End();
    }
    ImGui::PopStyleVar();

    if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
        ImGui::SetScrollHereY(1.0f);

    ImGui::EndChild();
    ImGui::End();
}

void Log::logInfo(const char* fmt, ...)
{
    addLog("[INFO] ");

    int old_size = buf.size();
    va_list args;
    va_start(args, fmt);
    buf.appendfv(fmt, args);
    va_end(args);

    addLog("\n");
}

void Log::logWarning(const char* fmt, ...)
{
    addLog("[WARN] ");

    int old_size = buf.size();
    va_list args;
    va_start(args, fmt);
    buf.appendfv(fmt, args);
    va_end(args);

    addLog("\n");
}

void Log::logError(const char* fmt, ...)
{
    addLog("[ERROR] ");

    int old_size = buf.size();
    va_list args;
    va_start(args, fmt);
    buf.appendfv(fmt, args);
    va_end(args);

    addLog("\n");
}