std::pair<ImVec2, ImVec2> centerImage(Buffer &image, ImVec2 dim = ImVec2(0,0)){
    ImGuiIO &io = ImGui::GetIO(); (void)io;

    float viewport_width = io.DisplaySize.x;
    float viewport_height = io.DisplaySize.y;

    // Calculate the center of the viewport
    float center_x = viewport_width / 2.0f;
    float center_y = viewport_height / 2.0f;

    // Use passed dimensions if they are non-zero, otherwise use image dimensions
    int input_width = (dim.x != 0) ? dim.x : image.info.width;
    int input_height = (dim.y != 0) ? dim.y : image.info.height;

    // Calculate scaling factors for the image to maintain aspect ratio
    float scale_width = viewport_width / static_cast<float>(input_width);
    float scale_height = viewport_height / static_cast<float>(input_height);
    float scale = std::min(scale_width, scale_height); // * zoomValue; // Use the smaller scale factor

    // Calculate the new dimensions of the image
    float scaled_width = input_width * scale;
    float scaled_height = input_height * scale;

    // Calculate the position to center the image
    ImVec2 image_pos = ImVec2(center_x - scaled_width / 2.0f, center_y - scaled_height / 2.0f);
    ImVec2 image_size = ImVec2(image_pos.x + scaled_width, image_pos.y + scaled_height);

    // printf("%.3f,%.3f | %.3f,%.3f\r\n", viewport_width, viewport_height, image_size.x, image_size.y);

    return std::make_pair(image_pos, image_size);
}