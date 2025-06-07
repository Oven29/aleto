#pragma once

#include <wx/grid.h>
#include <wx/wx.h>

#include "MainFrame.hpp"

namespace {

void addInputWithLabel(wxPanel* panel, wxTextCtrl* input, const wxString& textLabel) {
    wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
    auto* label = new wxStaticBox(panel, wxID_ANY, textLabel);
    sizer->Add(label, 0, wxCENTER, 8);
    sizer->Add(input, 1, wxALL, 8);
    panel->SetSizer(sizer);
};

}  // namespace

enum class ConnectType {
    SQLITE,
    POSTGRESQL
};

class FormFrame : public wxFrame {
 public:
    FormFrame() : wxFrame(nullptr, wxID_ANY, "aleto", wxDefaultPosition, wxSize(400, 600)) {
        wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
        wxPanel* panel = new wxPanel(this);

        // add connect type
        auto* typeConnectLabel = new wxStaticText(panel, wxID_ANY, wxT("Выберите тип базы данных:"));
        auto* typeConnectSizer = new wxBoxSizer(wxHORIZONTAL);
        auto* sqliteRadio = new wxRadioButton(panel, wxID_ANY, wxT("Sqlite"));
        auto* postgresqlRadio = new wxRadioButton(panel, wxID_ANY, wxT("PostgreSQL"));
        sqliteRadio->Bind(wxEVT_RADIOBUTTON, &FormFrame::onSqlite, this);
        postgresqlRadio->Bind(wxEVT_RADIOBUTTON, &FormFrame::onPostgres, this);
        typeConnectSizer->Add(sqliteRadio, 0, wxRIGHT, 8);
        typeConnectSizer->Add(postgresqlRadio, 0, wxRIGHT, 8);
        mainSizer->Add(typeConnectLabel, 0, wxCENTER, 10);
        mainSizer->Add(typeConnectSizer, 0, wxALL, 10);
        panel->SetSizer(mainSizer);
        sqliteRadio->SetValue(true);
        connectType = ConnectType::SQLITE;

        // create path panel
        wxBoxSizer* pathSizer = new wxBoxSizer(wxHORIZONTAL);
        pathPanel = new wxPanel(panel);
        auto* pathLabel = new wxStaticText(pathPanel, wxID_ANY, wxT("Путь к базе данных: "));
        auto* pathButton = new wxButton(pathPanel, wxID_ANY, wxT("Обзор"));
        fileDialog = new wxFileDialog(this, "Choose a file", "", "", "Database files (*.db)|*.db|Sqlite files (*.sqlite)|*.sqlite", wxFD_OPEN);
        selectPath = new wxStaticText(panel, wxID_ANY, wxT(""));
        pathButton->Bind(wxEVT_BUTTON, &FormFrame::onSelectPath, this);
        pathSizer->Add(pathLabel, 0, wxCENTER, 8);
        pathSizer->Add(pathButton, 1, wxRIGHT, 8);
        pathPanel->SetSizer(pathSizer);
        mainSizer->Add(pathPanel, 0, wxALL, 10);
        mainSizer->Add(selectPath, 1, wxRIGHT, 8);

        // create host panel
        hostPanel = new wxPanel(panel);
        hostInput = new wxTextCtrl(hostPanel, wxID_ANY);
        addInputWithLabel(hostPanel, hostInput, wxT("Хост: "));
        mainSizer->Add(hostPanel, 0, wxEXPAND | wxALL, 10);
        hostPanel->Enable(false);

        // create port panel
        portPanel = new wxPanel(panel);
        portInput = new wxTextCtrl(portPanel, wxID_ANY);
        addInputWithLabel(portPanel, portInput, wxT("Порт: "));
        mainSizer->Add(portPanel, 0, wxEXPAND | wxALL, 10);
        portPanel->Enable(false);

        // create user panel
        userPanel = new wxPanel(panel);
        userInput = new wxTextCtrl(userPanel, wxID_ANY);
        addInputWithLabel(userPanel, userInput, wxT("Пользователь: "));
        mainSizer->Add(userPanel, 0, wxEXPAND | wxALL, 10);
        userPanel->Enable(false);
        
        // create password panel
        passwordPanel = new wxPanel(panel);
        passwordInput = new wxTextCtrl(passwordPanel, wxID_ANY);
        addInputWithLabel(passwordPanel, passwordInput, wxT("Пароль: "));
        mainSizer->Add(passwordPanel, 0, wxEXPAND | wxALL, 10);
        passwordPanel->Enable(false);
        
        // create name panel
        namePanel = new wxPanel(panel);
        nameInput = new wxTextCtrl(namePanel, wxID_ANY);
        addInputWithLabel(namePanel, nameInput, wxT("Название: "));
        mainSizer->Add(namePanel, 0, wxEXPAND | wxALL, 10);
        namePanel->Enable(false);

        // create submit button
        wxButton* submitButton = new wxButton(panel, wxID_ANY, wxT("Подключиться"));
        submitButton->Bind(wxEVT_BUTTON, &FormFrame::onSubmit, this);
        mainSizer->Add(submitButton, 0, wxCENTER, 10);
    };

 private:
    wxPanel* pathPanel;
    wxFileDialog* fileDialog;
    wxStaticText* selectPath;

    ConnectType connectType;

    wxPanel* hostPanel;
    wxPanel* portPanel;
    wxPanel* userPanel;
    wxPanel* passwordPanel;
    wxPanel* namePanel;

    wxTextCtrl* hostInput;
    wxTextCtrl* portInput;
    wxTextCtrl* userInput;
    wxTextCtrl* passwordInput;
    wxTextCtrl* nameInput;

    void onPostgres(wxCommandEvent&) {
        pathPanel->Enable(false);
        hostPanel->Enable(true);
        portPanel->Enable(true);
        userPanel->Enable(true);
        passwordPanel->Enable(true);
        namePanel->Enable(true);
        selectPath->SetLabel(wxT(""));
        connectType = ConnectType::POSTGRESQL;
    }

    void onSqlite(wxCommandEvent&) {
        pathPanel->Enable(true);
        hostPanel->Enable(false);
        portPanel->Enable(false);
        userPanel->Enable(false);
        passwordPanel->Enable(false);
        namePanel->Enable(false);
        connectType = ConnectType::SQLITE;
    }

    void onSelectPath(wxCommandEvent&) {
        if (fileDialog->ShowModal() == wxID_OK) {
            selectPath->SetLabel(fileDialog->GetPath());
        }
    }

    void onSubmit(wxCommandEvent&) {
        std::unique_ptr<base::Database> db{};

        if (connectType == ConnectType::SQLITE) {
            wxString path = selectPath->GetLabel();
            if (path.empty()) {
                wxMessageBox(wxT("Выберите базу данных"), wxT("Подключение"), wxOK | wxICON_WARNING);
                return;
            }
            db = std::make_unique<sqlite::SQLiteDB>(path.ToStdString());
        } else {
            wxString host = hostInput->GetValue();
            wxString port = portInput->GetValue();
            wxString user = userInput->GetValue();
            wxString password = passwordInput->GetValue();
            wxString name = nameInput->GetValue();
            if (host.empty() || port.empty() || user.empty() || password.empty() || name.empty()) {
                wxMessageBox(wxT("Заполните все поля"), wxT("Подключение"), wxOK | wxICON_WARNING);
                return;
            }
            db = std::make_unique<postgresql::PostgreSqlDB>(host.ToStdString(), std::stoi(port.ToStdString()), user.ToStdString(),
                                                                       password.ToStdString(), name.ToStdString());

        }

        MainFrame* mainFrame = new MainFrame(std::move(db));
        mainFrame->Show();
        this->Destroy();
    }
};
