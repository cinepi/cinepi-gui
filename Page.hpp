#ifndef PAGE_HPP
#define PAGE_HPP

#pragma once

#include "imgui.h"

#include "Application.hpp"
#include "ImgAsset.hpp"
#include "EglBuffers.hpp"

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

class Page {
    public:
        Page(Application& app) :
            app(app)
        {} 
        virtual ~Page() {} 

        virtual void show() = 0;

    protected:
        std::shared_ptr<spdlog::logger> console;
        Application& app;

    private:
        
};

Page *make_overlays_page(Application& app);
Page *make_viewport_page(Application& app);
Page *make_menus_page(Application& app);

class Menus : public Page
{
    public:
        Menus(Application& app) : 
            Page(app),
            logo("cinepi-gui/assets/img/logo.png")
            {
                console = spdlog::stdout_color_mt("menus");
            }
        ~Menus() {}

        virtual void show() override;

    private:

        int menu_height = 96;

        void menu_top();
        void menu_bottom();

        ImgAsset logo;
        
};

class Overlays : public Page
{
    public:
        Overlays(Application& app) : 
            Page(app),
            en_cross(true), en_thirds(true)
            {
                console = spdlog::stdout_color_mt("overlays");
            }
        ~Overlays() {}

        virtual void show() override;

        bool en_cross;
        bool en_thirds;

    private:
        
};

class Viewport : public Page
{
    public:
        Viewport(Application& app) : 
            Page(app),
            yuv_preview("cinepi-gui/shaders/eglYuv.vert","cinepi-gui/shaders/eglYuv.frag")
            {
                console = spdlog::stdout_color_mt("viewport");
            }
        ~Viewport() {}

        void process();
        void init();

        virtual void show() override;

        Shader yuv_preview;

    private:
};
#endif // PAGE_HPP
