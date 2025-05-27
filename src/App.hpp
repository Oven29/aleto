#pragma once

#include <wx/wx.h>

#include "ui/FormFrame.hpp"

class MyApp : public wxApp {
 public:
    virtual bool OnInit() {
        FormFrame* frame = new FormFrame();
        frame->Show();
        return true;
    }
};
