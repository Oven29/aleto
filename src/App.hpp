#pragma once

#include <wx/wx.h>

#include "MainFrame.hpp"

class MyApp : public wxApp {
public:
  virtual bool OnInit() {
    MainFrame *frame = new MainFrame();
    frame->Show();
    return true;
  }
};
