#include "Page.hpp"

void Overlays::show()
{
    ImGuiIO &io = ImGui::GetIO(); (void)io;
    ImGui::SetNextWindowSize(io.DisplaySize);
    ImGui::SetNextWindowPos(ImVec2(0, 0));

    ImGuiWindowFlags window_flags = 0;
    window_flags |= ImGuiWindowFlags_NoTitleBar;
    window_flags |= ImGuiWindowFlags_NoScrollbar;
    window_flags |= ImGuiWindowFlags_NoDecoration;
    window_flags |= ImGuiWindowFlags_NoInputs;
    window_flags |= ImGuiWindowFlags_NoMove;
    window_flags |= ImGuiWindowFlags_NoResize;
    window_flags |= ImGuiWindowFlags_NoCollapse;
    window_flags |= ImGuiWindowFlags_NoNav;
    window_flags |= ImGuiWindowFlags_NoBackground;

    ImGui::Begin("Overlay", NULL, window_flags); 
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    if(en_cross)
    {
        const int line_width = 80;
        draw_list->AddLine(ImVec2(io.DisplaySize.x/2 - (line_width/2),io.DisplaySize.y/2), ImVec2(io.DisplaySize.x/2 + (line_width/2),io.DisplaySize.y/2), 0xFFFFFFFF);
        draw_list->AddLine(ImVec2(io.DisplaySize.x/2, io.DisplaySize.y/2 - (line_width/2)), ImVec2(io.DisplaySize.x/2, io.DisplaySize.y/2 + (line_width/2)), 0xFFFFFFFF);
    }
    if(en_thirds){
        draw_list->AddLine(ImVec2(io.DisplaySize.x/3, 0), ImVec2(io.DisplaySize.x/3, io.DisplaySize.y), 0xFFFFFFFF, 2.0f);
        draw_list->AddLine(ImVec2(io.DisplaySize.x/3 * 2, 0), ImVec2(io.DisplaySize.x/3 * 2, io.DisplaySize.y), 0xFFFFFFFF, 2.0f);
        draw_list->AddLine(ImVec2(0, io.DisplaySize.y/3), ImVec2(io.DisplaySize.x, io.DisplaySize.y/3), 0xFFFFFFFF, 2.0f);
        draw_list->AddLine(ImVec2(0, io.DisplaySize.y/3 * 2), ImVec2(io.DisplaySize.x, io.DisplaySize.y/3 * 2), 0xFFFFFFFF, 2.0f);
    }
    ImGui::End();
}

Page *make_overlays_page(Application& app){
    return new Overlays(app);
}