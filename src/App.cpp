#include "App.h"

namespace App {

bool App::OnInit() {
  MainFrame::MainFrame *frame = new MainFrame::MainFrame();
  frame->Show(true);
  return true;
}

} // namespace App
