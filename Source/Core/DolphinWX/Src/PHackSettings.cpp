// Copyright (C) 2003 Dolphin Project.

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, version 2.0.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License 2.0 for more details.

// A copy of the GPL 2.0 should have been included with the program.
// If not, see http://www.gnu.org/licenses/

// Official SVN repository and contact information can be found at
// http://code.google.com/p/dolphin-emu/

#include "PHackSettings.h"
#include "ConfigManager.h"

extern PHackData PHack_Data;

BEGIN_EVENT_TABLE(CPHackSettings, wxDialog)
	EVT_CHOICE(ID_PHACK_CHOICE, CPHackSettings::SetRefresh)
	EVT_CLOSE(CPHackSettings::OnClose)
	EVT_BUTTON(wxID_OK, CPHackSettings::SavePHackData)
END_EVENT_TABLE()

CPHackSettings::CPHackSettings(wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& position, const wxSize& size, long style)
	: wxDialog(parent, id, title, position, size, style)
{
	CreateGUIControls();
	std::string _iniFilename;
	_iniFilename = std::string(File::GetUserPath(D_GAMECONFIG_IDX)) + "PH_PRESETS.ini";
	PHPresetsIni.Load(_iniFilename.c_str());
	//PHPresetsIni.SortSections();
	//PHPresetsIni.Save(_iniFilename.c_str());

	LoadPHackData();
}

CPHackSettings::~CPHackSettings()
{
}

void CPHackSettings::CreateGUIControls()
{
	PHackChoiceText = new wxStaticText(this, ID_PHACK_CHOICE_TEXT, _("Presets: "), wxDefaultPosition, wxDefaultSize);
	PHackChoice = new wxChoice(this, ID_PHACK_CHOICE, wxDefaultPosition, wxDefaultSize, wxArrayString(0, wxString("", *wxConvCurrent)), 0, wxDefaultValidator);
	PHackChoice->SetToolTip(_("Load preset values from hack patterns available."));
	PHackZNearText = new wxStaticText(this, ID_PHACK_ZNEAR_TEXT, _("zNear Correction: "), wxDefaultPosition, wxDefaultSize);
	PHackZNear = new wxTextCtrl(this, ID_PHACK_ZNEAR, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator);
	PHackZNear->SetToolTip(_("Adds the specified value to zNear Parameter.\nTwo ways to express the floating point values.\nExample: entering '\'200'\' or '\'0.0002'\' directly, it produces equal effects, the acquired value will be '\'0.0002'\'.\nValues: (0->+/-Integer) or (0->+/-FP[6 digits of precision])\n\nNOTE: Check LogWindow/Console for the acquired values."));
	PHackSZNear = new wxCheckBox(this, ID_PHACK_SZNEAR, _("(-)+zNear"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE, wxDefaultValidator);
	PHackSZNear->SetToolTip(_("Changes sign to zNear Parameter (after correction)"));
	PHackZFarText = new wxStaticText(this, ID_PHACK_ZFAR_TEXT, _("zFar Correction: "), wxDefaultPosition, wxDefaultSize);
	PHackZFar = new wxTextCtrl(this, ID_PHACK_ZFAR, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator);
	PHackZFar->SetToolTip(_("Adds the specified value to zFar Parameter.\nTwo ways to express the floating point values.\nExample: entering '\'200'\' or '\'0.0002'\' directly, it produces equal effects, the acquired value will be '\'0.0002'\'.\nValues: (0->+/-Integer) or (0->+/-FP[6 digits of precision])\n\nNOTE: Check LogWindow/Console for the acquired values."));
	PHackSZFar = new wxCheckBox(this, ID_PHACK_SZFAR, _("(-)+zFar"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE, wxDefaultValidator);
	PHackSZFar->SetToolTip(_("Changes sign to zFar Parameter (after correction)"));
	PHackExP = new wxCheckBox(this, ID_PHACK_EXP, _("Extra Parameter"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE, wxDefaultValidator);
	PHackExP->SetToolTip(_("Extra Parameter useful in '\'Metroid: Other M'\' only."));

	wxStaticBoxSizer *sbPHackSettings = new wxStaticBoxSizer(wxVERTICAL, this, _("Parameters"));
	wxFlexGridSizer *szrPHackSettings = new wxFlexGridSizer(3, 5, 5);
	sbPHackSettings->Add(szrPHackSettings, 0, wxEXPAND|wxLEFT|wxTOP, 5);
	szrPHackSettings->Add(PHackZNearText, 0, wxALIGN_CENTER_VERTICAL);
	szrPHackSettings->Add(PHackZNear, 1, wxEXPAND);
	szrPHackSettings->Add(PHackSZNear, 0, wxEXPAND|wxLEFT, 5);
	szrPHackSettings->Add(PHackZFarText, 0, wxALIGN_CENTER_VERTICAL);
	szrPHackSettings->Add(PHackZFar, 1, wxEXPAND);
	szrPHackSettings->Add(PHackSZFar, 0, wxEXPAND|wxLEFT, 5);
	szrPHackSettings->Add(PHackExP, 0, wxEXPAND|wxTOP|wxBOTTOM, 5);

	wxBoxSizer* sPHackButtons = new wxBoxSizer(wxHORIZONTAL);
	wxButton* bOK = new wxButton(this, wxID_OK, _("OK"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator);
	wxButton* bCancel = new wxButton(this, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator);
	sPHackButtons->Add(0, 0, 1, wxEXPAND, 5);
	sPHackButtons->Add(bOK, 0, wxALL, 5);
	sPHackButtons->Add(bCancel, 0, wxALL, 5);
	
	wxBoxSizer* sPHack = new wxBoxSizer(wxVERTICAL);
	sPHack->Add(PHackChoiceText, 0, wxEXPAND|wxLEFT|wxRIGHT|wxTOP, 5);
	sPHack->Add(PHackChoice, 0, wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM, 5);
	sPHack->Add(sbPHackSettings, 0, wxEXPAND|wxALL, 5);
	sPHack->Add(sPHackButtons, 0, wxEXPAND, 5);

	SetSizerAndFit(sPHack);
}

void CPHackSettings::LoadPHackData()
{
	std::string sTemp;
	char sIndex[15];

	PHackChoice->Clear();
	PHackChoice->Append(_("[Custom]"));
	for (int i=0 ;  ; i++)
	{
		sprintf(sIndex,"%d",i);
		if (!PHPresetsIni.Exists(sIndex, "Title"))
			break;
		PHPresetsIni.Get(sIndex, "Title", &sTemp);
		if (sTemp.empty())
			sTemp = wxString(_("(UNKNOWN)")).char_str();
		if (i == 0)
			PHackChoice->Append(wxString("-----------", *wxConvCurrent));
		PHackChoice->Append(wxString(sTemp.c_str(), *wxConvCurrent));
	}
	PHackChoice->Select(0);

	PHackSZNear->Set3StateValue((wxCheckBoxState)PHack_Data.PHackSZNear);
	PHackSZFar->Set3StateValue((wxCheckBoxState)PHack_Data.PHackSZFar);
	PHackExP->Set3StateValue((wxCheckBoxState)PHack_Data.PHackExP);

	PHackZNear->SetValue(wxString(PHack_Data.PHZNear.c_str(), *wxConvCurrent));
	PHackZFar->SetValue(wxString(PHack_Data.PHZFar.c_str(), *wxConvCurrent));
}

void CPHackSettings::SetRefresh(wxCommandEvent& event)
{
	bool bTemp;
	std::string sTemp;
	char sIndex[15];
	int index = event.GetSelection();
	if (index > 1)
	{
		index -= 2;
		sprintf(sIndex,"%d", index);

		PHPresetsIni.Get(sIndex, "PH_SZNear", &bTemp);
		PHackSZNear->Set3StateValue((wxCheckBoxState)bTemp);
		PHPresetsIni.Get(sIndex, "PH_SZFar", &bTemp);
		PHackSZFar->Set3StateValue((wxCheckBoxState)bTemp);
		PHPresetsIni.Get(sIndex, "PH_ExtraParam", &bTemp);
		PHackExP->Set3StateValue((wxCheckBoxState)bTemp);
		PHPresetsIni.Get(sIndex, "PH_ZNear", &sTemp);
		PHackZNear->SetValue(wxString(sTemp.c_str(), *wxConvCurrent));
		PHPresetsIni.Get(sIndex, "PH_ZFar", &sTemp);
		PHackZFar->SetValue(wxString(sTemp.c_str(), *wxConvCurrent));
	}
}

void CPHackSettings::OnClose(wxCloseEvent& WXUNUSED (event))
{
	Destroy();
}

void CPHackSettings::SavePHackData(wxCommandEvent& WXUNUSED (event))
{
	PHack_Data.PHackSZNear = PHackSZNear->GetValue();
	PHack_Data.PHackSZFar = PHackSZFar->GetValue();
	PHack_Data.PHackExP = PHackExP->GetValue();

	PHack_Data.PHZNear = PHackZNear->GetValue().char_str();
	PHack_Data.PHZFar = PHackZFar->GetValue().char_str();

	AcceptAndClose();
}