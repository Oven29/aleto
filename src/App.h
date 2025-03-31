#pragma once

#include "MainFrame.h"
#include <wx/wx.h>

namespace App {

class App : public wxApp {
public:
  virtual bool OnInit() override;
};

} // namespace App
