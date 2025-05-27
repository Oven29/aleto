#pragma once

#include <wx/wx.h>

#include "ui/FormFrame.hpp"

class MyApp : public wxApp {
 public:
    virtual bool OnInit() {
        // MainFrame* frame = new MainFrame(std::move(std::make_unique<musoci::sqlite::Sqlite>("/home/ivan/projects/aleto/database.db")));
        FormFrame* frame = new FormFrame();
        frame->Show();
        return true;
    }
};
