/*
 * Copyright (C) Ascensio System SIA, 2009-2026
 *
 * This program is a free software product. You can redistribute it and/or
 * modify it under the terms of the GNU Affero General Public License (AGPL)
 * version 3 as published by the Free Software Foundation, together with the
 * additional terms provided in the LICENSE file.
 *
 * This program is distributed WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. For
 * details, see the GNU AGPL at: https://www.gnu.org/licenses/agpl-3.0.html
 *
 * You can contact Ascensio System SIA by email at info@onlyoffice.com
 * or by postal mail at 20A-6 Ernesta Birznieka-Upisha Street, Riga,
 * LV-1050, Latvia, European Union.
 *
 * The interactive user interfaces in modified versions of the Program
 * are required to display Appropriate Legal Notices in accordance with
 * Section 5 of the GNU AGPL version 3.
 *
 * No trademark rights are granted under this License.
 *
 * All non-code elements of the Product, including illustrations,
 * icon sets, and technical writing content, are licensed under the
 * Creative Commons Attribution-ShareAlike 4.0 International License:
 * https://creativecommons.org/licenses/by-sa/4.0/legalcode
 *
 * This license applies only to such non-code elements and does not
 * modify or replace the licensing terms applicable to the Program's
 * source code, which remains licensed under the GNU Affero General
 * Public License v3.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "uiwindow.h"
#include <future>

#define WINDOW_SIZE Size(768, 480)

class UILabel;
class UIProgressBar;
class UIWidget;
class UIBoxLayout;
class UICheckBox;
class UIRadioButton;
class UIButton;
class CDownloader;
class MainWindow : public UIWindow
{
public:
    MainWindow(UIWidget *parent, const Rect &rc);
    ~MainWindow();

    void initInstallationMode();
    void initControlMode(const std::wstring &path, const std::wstring &arch);

protected:
    virtual bool event(UINT, WPARAM, LPARAM, LRESULT*) override;

private:
    void startInstall();
    void finishInstall(const std::wstring &app_path);
    void startUpdate();
    // void startRepair();
    void startUninstall();
    void createSelectionPage();
    void createProgressPage(const std::wstring &text);
    void createCloseButton();
    void createCloseAndBackButtons();
    void runProcessAsync(const std::wstring &cmd, const std::wstring &args, const std::function<void(int)> &onComplete);
    std::wstring fillInstalledVerInfo();
    CDownloader* startDownload(const std::wstring &install_type, const std::wstring &arch, const std::wstring &path, const std::function<void()> &onComplete);

    enum class Mode : BYTE {Install, Control};
    enum Selectors : BYTE {UpdateRadio = 1, RepairRadio = 2, UninstRadio = 4, ClrDataCheck = 8, ClrStnCheck = 16, ClrAllCheck = 32, LaunchCheck = 64};
    std::future<void> m_future;
    std::wstring m_uninst_cmd,
                 m_ver,
                 m_arch,
                 m_package;
    UILabel     *m_comntLbl,
                *m_versionLbl,
                *m_comntInfoLbl;
    UIProgressBar *m_bar;
    UIWidget      *m_cenPanel;
    UIBoxLayout   *m_cenPanelVlut;
    UICheckBox    *m_launchCheck;
    UIRadioButton *m_updRadio,
                  // *m_repRadio,
                  *m_uninsRadio;
    UIButton      *m_cancelBtn;
    Mode m_mode;
    int  m_resize_conn;
    BYTE m_checkState;
    bool m_is_checked,
         m_is_completed;
};

#endif // MAINWINDOW_H
