#pragma once

#include <map>
#include <vector>
#include <wx/grid.h>
#include <wx/wx.h>

#include "../libs/musoci/driver.hpp"

namespace {

std::unique_ptr<musoci::base::Base> getDb() {
}

} // namespace

class MainFrame : public wxFrame {
public:
  MainFrame()
      : wxFrame(nullptr, wxID_ANY, "aleto", wxDefaultPosition,
                wxSize(1280, 720)),
        db(getDb()) {
    wxBoxSizer *mainSizer = new wxBoxSizer(wxHORIZONTAL);

    // Список таблиц
    tableList = new wxListBox(this, wxID_ANY);
    mainSizer->Add(tableList, 1, wxEXPAND | wxALL, 5);

    // Таблица
    grid = new wxGrid(this, wxID_ANY);
    grid->CreateGrid(0, 0);
    mainSizer->Add(grid, 3, wxEXPAND | wxALL, 5);

    SetSizer(mainSizer);

    tables = {};

    auto data = std::dynamic_pointer_cast<types::SchemaListData>(db->connect());

    for (const auto &item : data->items) {
      std::vector<std::string> t;
      for (const auto &c : item.columns) {
        t.push_back(c->name);
      }
      std::vector<std::vector<std::string>> d;
      d.push_back(t);
      auto tableData =
          std::dynamic_pointer_cast<types::TableData>(db->get(item.title));
      for (const auto &c : tableData->data) {
        d.push_back(c);
      }
      tables[item.title] = d;
    }
    // std::cout << *items << std::endl;

    // Заполняем список таблиц
    for (const auto &pair : tables) {
      tableList->Append(pair.first);
    }

    tableList->Bind(wxEVT_LISTBOX, &MainFrame::OnTableSelected, this);
  }

private:
  wxListBox *tableList;
  wxGrid *grid;

  std::map<std::string, std::vector<std::vector<std::string>>> tables;
  std::unique_ptr<musoci::base::Base> db;

  void OnTableSelected(wxCommandEvent &event) {
    std::string tableName = tableList->GetStringSelection().ToStdString();
    const auto &data = tables[tableName];

    if (data.empty())
      return;

    grid->ClearGrid();
    if (grid->GetNumberRows() > 0)
      grid->DeleteRows(0, grid->GetNumberRows());
    if (grid->GetNumberCols() > 0)
      grid->DeleteCols(0, grid->GetNumberCols());

    int cols = data[0].size();
    int rows = data.size() - 1;

    grid->AppendCols(cols);
    grid->AppendRows(rows);

    // Заголовки
    for (int col = 0; col < cols; ++col) {
      grid->SetColLabelValue(col, data[0][col]);
    }

    // Данные
    for (int row = 0; row < rows; ++row) {
      for (int col = 0; col < cols; ++col) {
        grid->SetCellValue(row, col, data[row + 1][col]);
      }
    }
  }
};
