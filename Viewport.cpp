#include "Page.hpp"
#include "Utilities.hpp"

#include <utility>

std::pair<int, int> calculateNewImageSize(int screenWidth, int screenHeight, int imageWidth, int imageHeight) {
    // Calculate the scaling factor for both dimensions
    double widthScale = static_cast<double>(screenWidth) / imageWidth;
    double heightScale = static_cast<double>(screenHeight) / imageHeight;

    // Use the smaller scaling factor to keep the image within screen bounds
    double scale = std::min(widthScale, heightScale);

    // Calculate the new dimensions
    int newWidth = static_cast<int>(imageWidth * scale);
    int newHeight = static_cast<int>(imageHeight * scale);

    return std::make_pair(newWidth, newHeight);
}

void Viewport::process()
{
    FrameBuffer fb = app.buffers.getBuffer();

    if(!yuv_preview.initialized){
        auto [newWidth, newHeight] = calculateNewImageSize(app.app_width, app.app_height, fb.isp.info.width, fb.isp.info.height);
        yuv_preview.setup(newWidth, newHeight, GL_LINEAR);
    } else {
        glBindFramebuffer(GL_FRAMEBUFFER, yuv_preview.frameBuffer);
        glViewport(0, 0, yuv_preview.width, yuv_preview.height);  

        // Clear the framebuffer
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(yuv_preview.program);

        // Bind the texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_EXTERNAL_OES, fb.isp.texture);
        glUniform1i(glGetUniformLocation(yuv_preview.program, "tex"), 0);

        glBindVertexArray(yuv_preview.quadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);  // Unbind FBO
    }
}

void Viewport::show()
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

    FrameBuffer fb = app.buffers.getBuffer();

    ImGui::Begin("Viewport", NULL, window_flags); 

    auto [pos, scale] = centerImage(fb.isp);
    ImGui::GetBackgroundDrawList()->AddImage(
        (void*)(intptr_t)yuv_preview.texture,
        pos,
        scale,
        ImVec2(0, 0),
        ImVec2(1, 1)
    );

    ImGui::End();
}

Page *make_viewport_page(Application& app){
    return new Viewport(app);
}