#pragma once

#include <wx/grid.h>
#include <wx/wx.h>
#include <map>
#include <stdexcept>
#include <vector>

#include "../../libs/musoci/postgresql.hpp"
#include "../../libs/musoci/sqlite.hpp"
#include "../core/config.hpp"

class MainFrame : public wxFrame {
 public:
    MainFrame(std::unique_ptr<base::Database> _db)
        : wxFrame(nullptr, wxID_ANY, wxT("aleto"), wxDefaultPosition, wxSize(config::WIDTH, config::HEIGHT),
                  wxDEFAULT_FRAME_STYLE & ~(wxRESIZE_BORDER | wxMAXIMIZE_BOX)),
          db(std::move(_db)) {
        std::vector<types::TableSchema> schemas{};
        try {
            schemas = db->getTables();
        } catch (const std::exception& e) {
            wxMessageBox(wxString::FromUTF8(e.what()), wxT("Подключение"), wxOK | wxICON_WARNING);
        }

        if (schemas.size() == 0) {
            wxMessageBox(wxT("База данных пуста"), wxT("Подключение"), wxOK | wxICON_WARNING);
        }

        wxBoxSizer* mainSizer = new wxBoxSizer(wxHORIZONTAL);
        wxPanel* panel = new wxPanel(this);
        panel->SetSizer(mainSizer);

        tableList = new wxListBox(panel, wxID_ANY);
        lastPages = std::map<std::string, int>();
        for (const auto& schema : schemas) {
            tableList->Append(wxString::FromUTF8(schema.title));
            lastPages[schema.title] = 1000000;
        }
        tableList->Bind(wxEVT_LISTBOX, &MainFrame::onTableSelected, this);
        mainSizer->Add(tableList, 1, wxEXPAND | wxALL, 5);

        // Правая часть
        wxBoxSizer* rightSizer = new wxBoxSizer(wxVERTICAL);
        wxPanel* rightPanel = new wxPanel(panel);
        mainSizer->Add(rightPanel, 3, wxEXPAND | wxALL, 5);

        wxBoxSizer* controlSizer = new wxBoxSizer(wxHORIZONTAL);
        wxButton* refreshDataButton = new wxButton(rightPanel, wxID_ANY, wxT("Обновить"));
        refreshDataButton->Bind(wxEVT_BUTTON, &MainFrame::refreshData, this);
        controlSizer->Add(refreshDataButton, 0, wxRIGHT, 8);
        rightSizer->Add(controlSizer, 0, wxALL, 8);

        // Добавляем grid внутрь rightSizer
        grid = new wxGrid(rightPanel, wxID_ANY);
        grid->CreateGrid(0, 0);
        // grid->SetRowLabelSize(0);
        grid->EnableEditing(true);
        grid->Bind(wxEVT_GRID_CELL_CHANGED, &MainFrame::onCellChanged, this);
        rightSizer->Add(grid, 1, wxEXPAND | wxALL, 0);
        grid->Bind(wxEVT_GRID_LABEL_LEFT_CLICK, &MainFrame::onColumnHeaderClick, this);

        // Панель и сайзер для навигации
        wxBoxSizer* navigationSizer = new wxBoxSizer(wxHORIZONTAL);
        wxPanel* navigationPanel = new wxPanel(rightPanel);
        wxButton* prevButton = new wxButton(navigationPanel, wxID_ANY, wxT("<"));
        wxButton* nextButton = new wxButton(navigationPanel, wxID_ANY, wxT(">"));
        wxButton* goButton = new wxButton(navigationPanel, wxID_ANY, wxT("GO"));
        pageText = new wxTextCtrl(navigationPanel, wxID_ANY, wxT("1"));

        prevButton->Bind(wxEVT_BUTTON, &MainFrame::onPrevPage, this);
        nextButton->Bind(wxEVT_BUTTON, &MainFrame::onNextPage, this);
        goButton->Bind(wxEVT_BUTTON, &MainFrame::goToPage, this);

        navigationSizer->Add(prevButton, 3, wxEXPAND | wxALL, 10);
        navigationSizer->Add(pageText, 2, wxCENTER, 10);
        navigationSizer->Add(goButton, 1, wxEXPAND | wxALL, 10);
        navigationSizer->Add(nextButton, 3, wxEXPAND | wxALL, 10);
        navigationPanel->SetSizer(navigationSizer);

        rightSizer->Add(navigationPanel, 0, wxEXPAND | wxALL, 5);

        // Присваиваем сайзер панели
        rightPanel->SetSizer(rightSizer);

        if (schemas.size() != 0) {
            loadPage(schemas[0].title);
        }
    }

 private:
    std::unique_ptr<base::Database> db;

    std::string currentTable;
    int currentPage;
    std::map<std::string, int> lastPages;
    std::map<std::tuple<int, int>, std::string> editedCells{};
    std::map<int, bool> sortAscending{};

    wxGrid* grid;
    wxListBox* tableList;
    wxTextCtrl* pageText;

    void onTableSelected(wxCommandEvent& event) {
        std::string tableName = tableList->GetStringSelection().ToStdString();
        loadPage(tableName);
    }

    void onPrevPage(wxCommandEvent&) { loadPage(currentTable, currentPage - 1); }

    void onNextPage(wxCommandEvent&) { loadPage(currentTable, currentPage + 1); }

    void goToPage(wxCommandEvent&) {
        int page = std::stoi(pageText->GetValue().ToStdString());
        if (page > 0 && page != currentPage) {
            loadPage(currentTable, page);
        }
    }

    void onCellChanged(wxGridEvent& event) {
        int row = event.GetRow();
        int col = event.GetCol();

        wxString newValue = grid->GetCellValue(row, col);
        grid->SetCellBackgroundColour(row, col, wxColour(255, 255, 153));

        editedCells[std::make_tuple(row, col)] = newValue.ToStdString();

        event.Skip();
    }

    void refreshData(wxCommandEvent&) { loadPage(currentTable, currentPage); }

    void loadPage(std::string tableName, int page = 1) {
        if (page <= 0 || page > lastPages[tableName]) {
            return;
        }

        types::TableData data{};
        try {
            data = db->select(tableName, (page - 1) * config::ROWS_ON_PAGE, config::ROWS_ON_PAGE);
            if (data.data.size() < config::ROWS_ON_PAGE && page < lastPages[tableName]) {
                lastPages[tableName] = page;
            }
            if (data.data.size() == 0) {
                throw std::runtime_error("Пустая страница");
            }
        } catch (const std::exception& e) {
            wxMessageBox(wxString::FromUTF8(e.what()), wxT("Подключение"), wxOK | wxICON_WARNING);
            return;
        }

        pageText->SetValue(wxString(std::to_string(page)));
        currentPage = page;
        currentTable = tableName;

        grid->ClearGrid();
        if (grid->GetNumberRows() != 0) {
            grid->DeleteRows(0, grid->GetNumberRows());
        }
        if (grid->GetNumberCols() != 0) {
            grid->DeleteCols(0, grid->GetNumberCols());
        }
        grid->AppendCols(data.columns.size());
        grid->AppendRows(data.data.size());
        for (int i = 0; i < data.columns.size(); i++) {
            grid->SetColLabelValue(i, wxString::FromUTF8(data.columns[i].name));
        }
        for (int i = 0; i < data.data.size(); i++) {
            for (int j = 0; j < data.data[i].size(); j++) {
                grid->SetCellValue(i, j, wxString::FromUTF8(data.data[i][j]));
            }
        }
    }

    void onColumnHeaderClick(wxGridEvent& event) {
        int col = event.GetCol();
        if (col < 0) {
            return;
        }
        // Сохраняем выбранную колонку и направление сортировки
        bool ascending = sortAscending[col] = !sortAscending[col];  // true = по возрастанию

        const char* upArrow = " >";
        const char* downArrow = " <";

        // Обновление заголовков
        for (int i = 0; i < grid->GetNumberCols(); ++i) {
            wxString label = grid->GetColLabelValue(i);

            // Удаляем символы сортировки
            label.Replace(upArrow, "");
            label.Replace(downArrow, "");

            // Добавляем значок только к текущей колонке
            if (i == col) {
                label += ascending ? upArrow : downArrow;
            }

            grid->SetColLabelValue(i, wxString::FromUTF8(label));
        }
        grid->ForceRefresh();

        int rows = grid->GetNumberRows();
        std::vector<std::pair<wxString, int>> data;

        for (int i = 0; i < rows; ++i) {
            data.emplace_back(grid->GetCellValue(i, col), i);  // значение, индекс
        }

        std::sort(data.begin(), data.end(), [ascending](const auto& a, const auto& b) { return ascending ? a.first < b.first : a.first > b.first; });

        // Копируем строки во временное хранилище
        std::vector<std::vector<wxString>> sortedRows;
        for (const auto& [_, index] : data) {
            std::vector<wxString> row;
            for (int c = 0; c < grid->GetNumberCols(); ++c) {
                row.push_back(grid->GetCellValue(index, c));
            }
            sortedRows.push_back(std::move(row));
        }

        // Обновляем строки в гриде
        for (int i = 0; i < rows; ++i) {
            for (int c = 0; c < grid->GetNumberCols(); ++c) {
                grid->SetCellValue(i, c, sortedRows[i][c]);
            }
        }
    }
};
