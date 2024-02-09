#include "Application.hpp"

#include <stdexcept>

void Application::init(unsigned int w = 1280, unsigned int h = 720){
    app_width = w;
    app_height = h;

    SDL_SetHint(SDL_HINT_KMSDRM_DEVICE_INDEX, "1");

    // Setup SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0)
    {
        throw std::runtime_error(SDL_GetError());
    }

    // GL ES 3.0 + GLSL 100
    const char* glsl_version = "#version 300 es";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_SetHint(SDL_HINT_OPENGL_ES_DRIVER, "1");

    // Create window with graphics context
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    // SDL_GL_SetAttribute(SDL_GL_FRAMEBUFFER_SRGB_CAPABLE, 1);
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI);
    // for fullscreen with no window decorations
    // window_flags = (SDL_WindowFlags)(window_flags | SDL_WINDOW_FULLSCREEN | SDL_WINDOW_BORDERLESS | SDL_WINDOW_FULLSCREEN_DESKTOP);
    window = SDL_CreateWindow("CINEPI", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, w, h, window_flags);
    gl_context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, gl_context);
    SDL_GL_SetSwapInterval(1); // Enable vsync

    char main_font_path[] = "cinepi-gui/assets/font/ENGCAPS.TTF";
    char icon_font_path[] = "cinepi-gui/assets/font/fontawesome-webfont.ttf";

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO &io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    // Disable .ini file saving
    io.IniFilename = NULL;

    io.Fonts->AddFontFromFileTTF(main_font_path, 60);
    ui8 = io.Fonts->AddFontFromFileTTF(main_font_path, 8);
    ui16 = io.Fonts->AddFontFromFileTTF(main_font_path, 16);
    ui24 = io.Fonts->AddFontFromFileTTF(main_font_path, 24);
    ui36 = io.Fonts->AddFontFromFileTTF(main_font_path, 36);
    ui48 = io.Fonts->AddFontFromFileTTF(main_font_path, 48);

    static const ImWchar icon_ranges[] = {ICON_MIN_FA, ICON_MAX_FA, 0};
    ImFontConfig icons_config1;
    icons_config1.MergeMode = true;
    icons_config1.PixelSnapH = true;
    icons_config1.OversampleH = 4;
    icons_config1.OversampleV = 4;

    ImFontConfig icons_config2;
    icons_config2.MergeMode = true;
    icons_config2.PixelSnapH = true;
    icons_config2.OversampleH = 4;
    icons_config2.OversampleV = 4;

    ImFontConfig icons_config3;
    icons_config3.MergeMode = true;
    icons_config3.PixelSnapH = true;
    icons_config3.OversampleH = 4;
    icons_config3.OversampleV = 4;

    icons_font24 = io.Fonts->AddFontFromFileTTF(icon_font_path, 24.0f, &icons_config1, icon_ranges);
    icons_font36 = io.Fonts->AddFontFromFileTTF(icon_font_path, 36.0f, &icons_config2, icon_ranges);
    icons_font48 = io.Fonts->AddFontFromFileTTF(icon_font_path, 48.0f, &icons_config3, icon_ranges);

    ImGui::StyleColorsDark();

    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init(glsl_version);

    buffers.init();
}

void Application::cleanup(){
    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void Application::beginDraw(){
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
}

void Application::internal_overlay(){
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

    ImGui::Begin("internal_overlay", NULL, window_flags); 
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    
    // Calculate the position for the text
    const char* version_text = "CINEPI-GUI v0.0.1";
    ImVec2 text_size = ImGui::CalcTextSize(version_text);
    ImVec2 text_pos = ImVec2(
        io.DisplaySize.x - text_size.x - (-192),  // X position: window width - text width - padding
        io.DisplaySize.y - text_size.y - (-32)   // Y position: window height - text height - padding
    );

    draw_list->AddText(ui16, ui16->FontSize, text_pos, IM_COL32(255, 255, 255, 128), version_text);
    ImGui::End();
}


void Application::endDraw(){
    // Rendering
    ImGuiIO &io = ImGui::GetIO(); (void)io;
    internal_overlay();
    ImGui::Render();
    glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    SDL_GL_SwapWindow(window);
}

