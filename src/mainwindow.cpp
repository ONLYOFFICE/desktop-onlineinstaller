#include "mainwindow.h"
#include "uiapplication.h"
#include "utils.h"
#include "uistyle.h"
#include "uithread.h"
#include "uicheckbox.h"
#include "uibutton.h"
#include "uilabel.h"
#include "uiboxlayout.h"
#include "uiradiobutton.h"
#include "uiprogressbar.h"
#include "uimetrics.h"
#include "uicaption.h"
#include "resource.h"
#include "translator.h"
#include "cdownloader.h"
#include "uiutils.h"
#include "cjson.h"
#include <Msi.h>
#include <ShlObj.h>
#include <Shlwapi.h>
#include <numeric>
#include "../desktop-apps/win-linux/src/defines.h"
#include "../desktop-apps/win-linux/src/prop/defines_p.h"


static wstring displayNameAddInfo(const wstring &disp_name, const wstring &info)
{
    size_t lbr = disp_name.find_last_of(L'(');
    size_t rbr = disp_name.find_last_of(L')');
    if (lbr != wstring::npos && rbr != wstring::npos && lbr < rbr) {
        wstring inside_brackets = disp_name.substr(lbr + 1, rbr - lbr - 1);
        if (inside_brackets.find(info) == wstring::npos) {
            inside_brackets += L" " + info;
        }
        return disp_name.substr(0, lbr + 1) + inside_brackets + disp_name.substr(rbr);
    }
    return disp_name + L" (" + info + L")";
}

MainWindow::MainWindow(UIWidget *parent, const Rect &rc) :
    UIWindow(parent, rc, RemoveSystemDecoration),
    m_comntLbl(nullptr),
    m_versionLbl(nullptr),
    m_comntInfoLbl(nullptr),
    m_bar(nullptr),
    m_launchCheck(nullptr),
    m_updRadio(nullptr),
    // m_repRadio(nullptr),
    m_uninsRadio(nullptr),
    m_cancelBtn(nullptr),
    m_mode(Mode::Install),
    m_resize_conn(0),
    m_checkState(UpdateRadio | LaunchCheck),
    m_is_checked(false),
    m_is_completed(false)
{
    setObjectGroupId(_T("MainWindow"));
    setWindowTitle(_TR(CAPTION));
    setIcon(IDI_MAINICON);
    if (UIUtils::winVersion() > UIUtils::WinXP && UIUtils::winVersion() < UIUtils::Win10)
        metrics()->setMetrics(Metrics::BorderWidth, 1);

    UIWidget *cw = new UIWidget(this);
    cw->setObjectGroupId(_T("MainWindow"));
    setCentralWidget(cw);
    setContentsMargins(0,0,0,0);

    UIBoxLayout *cenVlut = new UIBoxLayout(UIBoxLayout::Vertical);
    cenVlut->setContentMargins(0, 0, 0, 0);
    cenVlut->setSpacing(0);
    cw->setLayout(cenVlut);

    /* Caption section*/   
    UICaption *cap = new UICaption(cw);
    cap->setObjectGroupId(_T("HeaderBar"));
    cap->setResizingAvailable(false);
    cenVlut->addWidget(cap);

    UIBoxLayout *topHlut = new UIBoxLayout(UIBoxLayout::Horizontal, UILayout::AlignHRight | UILayout::AlignVCenter);
    topHlut->setContentMargins(0, 0, 0, 0);
    topHlut->setSpacing(0);
    cap->setLayout(topHlut);

    UIButton *closeBtn = new UIButton(cap);
    closeBtn->setObjectGroupId(_T("HeaderCloseButton"));
    closeBtn->setStockIcon(UIButton::CloseIcon);
    closeBtn->setIconSize(10, 10);
    closeBtn->onClick([this]() {
        close();
    });
    topHlut->addWidget(closeBtn);

    /* Central section */
    m_cenPanel = new UIWidget(cw);
    m_cenPanel->setObjectGroupId(_T("MainWindow"));
    cenVlut->addWidget(m_cenPanel);

    m_cenPanelVlut = new UIBoxLayout(UIBoxLayout::Vertical);
    m_cenPanelVlut->setContentMargins(6, 12, 6, 48);
    m_cenPanelVlut->setSpacing(6);
    m_cenPanel->setLayout(m_cenPanelVlut);
}

MainWindow::~MainWindow()
{
    if (m_future.valid())
        m_future.wait();
}

void MainWindow::initInstallationMode()
{
    m_is_checked = true;
    m_mode = Mode::Install;
    UIStyle *style = UIApplication::instance()->style();
    /* Image section*/
    UILabel *wlcLbl = new UILabel(m_cenPanel);
    wlcLbl->setObjectGroupId(_T("MainWindow"));
    wlcLbl->setBaseSize(282, 200);
    wlcLbl->setEMFIcon(style->themeId() == _T("Dark") ? IDI_WELCOME_DARK : IDI_WELCOME, 282, 200);
    wlcLbl->setSizePolicy(SizePolicy::HSizeBehavior, SizePolicy::Expanding);
    wlcLbl->setSizePolicy(SizePolicy::VSizeBehavior, SizePolicy::Fixed);
    m_cenPanelVlut->addWidget(wlcLbl);

    /* Check box section*/
    UICheckBox *chkBox = new UICheckBox(m_cenPanel, _TR(CHECK_SILENT));
    chkBox->setObjectGroupId(_T("Selector"));
    chkBox->setChecked(m_is_checked);
    chkBox->adjustSizeBasedOnContent();
    int chkMargin = 2 + (chkBox->metrics()->value(Metrics::IconWidth) + chkBox->metrics()->value(Metrics::TextMarginLeft))/2;
    chkBox->move(chkMargin * m_dpi_ratio + m_cenPanel->size().width/2 - chkBox->size().width/2, 254 * m_dpi_ratio);
    chkBox->onClick([chkBox, this]() {
        m_is_checked = chkBox->isChecked();
    });

    /* Comment section */
    wstring warn_text = _TR(LABEL_WARN_CLOSE);
    NS_Utils::Replace(warn_text, L"%1", _T(WINDOW_NAME));
    UILabel *comntLbl = new UILabel(m_cenPanel);
    comntLbl->setObjectGroupId(_T("Label"));
    comntLbl->setText(warn_text, true);
    comntLbl->setGeometry(0, m_cenPanel->size().height - 136 * m_dpi_ratio, m_cenPanel->size().width, 48 * m_dpi_ratio);

    /* Install button section */
    UIButton *instlBtn = new UIButton(m_cenPanel);
    instlBtn->setObjectGroupId(_T("PushButton"));
    instlBtn->setText(_TR(BUTTON_INSTALL));
    instlBtn->move(m_cenPanel->size().width/2 - 50 * m_dpi_ratio, m_cenPanel->size().height - 76 * m_dpi_ratio);
    instlBtn->onClick([=]() {
        m_cenPanel->disconnect(m_resize_conn);
        chkBox->close();
        comntLbl->close();
        instlBtn->close();
        startInstall();
    });

    m_resize_conn = m_cenPanel->onResize([this, chkBox, comntLbl, instlBtn, chkMargin](int w, int h) {
        chkBox->move(chkMargin * m_dpi_ratio + w/2 - chkBox->size().width/2, 254 * m_dpi_ratio);
        comntLbl->setGeometry(0, h - 136 * m_dpi_ratio, w, 48 * m_dpi_ratio);
        instlBtn->move(w/2 - 50 * m_dpi_ratio, h - 76 * m_dpi_ratio);
    });
}

void MainWindow::initControlMode(const std::wstring &_arch)
{
    m_mode = Mode::Control;
    /* Comment section */
    m_versionLbl = new UILabel(m_cenPanel);
    m_versionLbl->setObjectGroupId(_T("ControlLabel"));
    m_versionLbl->setText(fillInstalledVerInfo());
    m_versionLbl->setSizePolicy(SizePolicy::HSizeBehavior, SizePolicy::Expanding);
    m_versionLbl->setSizePolicy(SizePolicy::VSizeBehavior, SizePolicy::Fixed);
    m_versionLbl->metrics()->setMetrics(Metrics::TextMarginLeft, 12);
    m_cenPanelVlut->setContentMargins(18, 6, 6, 6);
    m_cenPanelVlut->addWidget(m_versionLbl);

    if (m_package == _TR(LABEL_UNKN_PACK) || m_ver == _TR(LABEL_UNKN_VER) || _arch.empty() || m_arch != _arch) {
        UILabel *errLbl = new UILabel(m_cenPanel);
        errLbl->setObjectGroupId(_T("ControlLabel"));
        errLbl->setText(_TR(LABEL_NO_OPTIONS));
        errLbl->setSizePolicy(SizePolicy::HSizeBehavior, SizePolicy::Expanding);
        errLbl->setSizePolicy(SizePolicy::VSizeBehavior, SizePolicy::Expanding);
        errLbl->metrics()->setMetrics(Metrics::TextMarginLeft, 24);
        m_cenPanelVlut->addWidget(errLbl);
        return;
    }
    createSelectionPage();
}

bool MainWindow::event(UINT msg, WPARAM wParam, LPARAM lParam, LRESULT *result)
{
    switch (msg) {
    case WM_DPICHANGED: {
        setResizable(true);
        setMinimumSize(0, 0);
        UIWindow::event(msg, wParam, lParam, result);
        setMinimumSize(WINDOW_SIZE.width * m_dpi_ratio, WINDOW_SIZE.height * m_dpi_ratio);
        setResizable(false);
        return true;
    }
    default:
        break;
    }
    return UIWindow::event(msg, wParam, lParam, result);
}

void MainWindow::startInstall()
{
    /* Comment section */
    m_comntLbl = new UILabel(m_cenPanel);
    m_comntLbl->setObjectGroupId(_T("Label"));
    m_comntLbl->setText(_TR(LABEL_DOWNLOAD), true);
    m_comntLbl->setGeometry(0, m_cenPanel->size().height - 161 * m_dpi_ratio, m_cenPanel->size().width, 28 * m_dpi_ratio);
    m_comntLbl->setFont(L"Segoe UI", 12);

    m_comntInfoLbl = new UILabel(m_cenPanel);
    m_comntInfoLbl->setObjectGroupId(_T("Label"));
    m_comntInfoLbl->setText(_TR(LABEL_ALMOST_DONE), true);
    m_comntInfoLbl->setGeometry(0, m_cenPanel->size().height - 122 * m_dpi_ratio, m_cenPanel->size().width, 40 * m_dpi_ratio);

    /* Progress section */
    m_bar = new UIProgressBar(m_cenPanel);
    m_bar->setObjectGroupId(_T("ProgressBar"));
    m_bar->setGeometry(0, m_cenPanel->size().height - 126 * m_dpi_ratio, m_cenPanel->size().width, 5 * m_dpi_ratio);
    m_bar->metrics()->setMetrics(Metrics::IconMarginLeft, 108);
    m_bar->metrics()->setMetrics(Metrics::IconMarginRight, 108);

    m_resize_conn = m_cenPanel->onResize([this](int w, int h) {
        m_comntLbl->setGeometry(0, h - 161 * m_dpi_ratio, w, 24 * m_dpi_ratio);
        m_comntInfoLbl->setGeometry(0, h - 122 * m_dpi_ratio, w, 40 * m_dpi_ratio);
        m_bar->setGeometry(0, m_cenPanel->size().height - 126 * m_dpi_ratio, m_cenPanel->size().width, 5 * m_dpi_ratio);
    });
    m_comntLbl->show();
    m_comntInfoLbl->show();
    m_bar->show();

    wstring path = NS_File::generateTmpFileName(L".exe");
    startDownload(L"iss", NS_Utils::IsWin64() ? _T("x64") : _T("x86"), path, [=]() {
            wstring args;
            if (m_is_checked) {
                args = _T("/VERYSILENT");
                m_comntLbl->setText(_TR(LABEL_INSTALL), true);
                m_bar->pulse(true);
            } else {
                hide();
            }
            m_future = std::async(std::launch::async, [=]() {
            DWORD status = NS_File::runProcess(path, args);
            UIThread::invoke(this, [=]() {
            if (status != 0) {
                DWORD err = GetLastError();
                if (!m_is_checked)
                    show();
                m_bar->pulse(false);
                m_bar->setProgress(0);
                m_comntInfoLbl->setText(err == 0 ? _TR(LABEL_ERR_RUNNING) + wstring(L" Exit code: ") + to_wstring(status) : NS_Utils::GetLastErrorAsString(err), true);
            } else {
                if (m_is_checked) {
                    wstring app_path;
                    if (NS_Utils::IsAppInstalled(app_path)) {
                        m_cenPanel->disconnect(m_resize_conn);
                        m_comntLbl->close();
                        m_comntInfoLbl->close();
                        m_bar->close();
                        UIThread::invoke(this, &MainWindow::finishInstall, app_path);
                    } else {
                        m_bar->pulse(false);
                        m_bar->setProgress(0);
                        m_comntLbl->setText(_TR(LABEL_ERR_INSTALL), true);
                    }
                } else {
                    close();
                }
            }
            if (NS_File::fileExists(path))
                NS_File::removeFile(path);
            if (m_mode == Mode::Control)
                createCloseAndBackButtons();
            });
            });
        });
}

void MainWindow::finishInstall(const std::wstring &app_path)
{
    /* Check box section*/
    m_is_checked = true;
    UICheckBox *chkBox = new UICheckBox(m_cenPanel, _TR(CHECK_LAUNCH));
    chkBox->setObjectGroupId(_T("Selector"));
    chkBox->setChecked(m_is_checked);
    chkBox->adjustSizeBasedOnContent();
    int chkMargin = 2 + (chkBox->metrics()->value(Metrics::IconWidth) + chkBox->metrics()->value(Metrics::TextMarginLeft))/2;
    chkBox->move(chkMargin * m_dpi_ratio + m_cenPanel->size().width/2 - chkBox->size().width/2, 254 * m_dpi_ratio);
    chkBox->onClick([chkBox, this]() {
        m_is_checked = chkBox->isChecked();
    });

    /* Comment section */
    wstring compl_text = _TR(LABEL_INSTALL_COMPL);
    NS_Utils::Replace(compl_text, L"%1", _T(WINDOW_NAME));
    UILabel *comntLbl = new UILabel(m_cenPanel);
    comntLbl->setObjectGroupId(_T("Label"));
    comntLbl->setText(compl_text, true);
    comntLbl->setGeometry(0, m_cenPanel->size().height - 136 * m_dpi_ratio, m_cenPanel->size().width, 48 * m_dpi_ratio);

    /* Install button section */
    UIButton *closeBtn = new UIButton(m_cenPanel);
    closeBtn->setObjectGroupId(_T("PushButton"));
    closeBtn->setText(_TR(BUTTON_CLOSE));
    closeBtn->move(m_cenPanel->size().width/2 - 50 * m_dpi_ratio, m_cenPanel->size().height - 76 * m_dpi_ratio);
    closeBtn->onClick([=]() {
        if (m_is_checked)
            NS_File::runProcess(L"explorer.exe", app_path + _T(APP_LAUNCH_NAME), false);
        close();
    });

    m_resize_conn = m_cenPanel->onResize([this, chkBox, comntLbl, closeBtn, chkMargin](int w, int h) {
        chkBox->move(chkMargin * m_dpi_ratio + w/2 - chkBox->size().width/2, 254 * m_dpi_ratio);
        comntLbl->setGeometry(0, h - 136 * m_dpi_ratio, w, 48 * m_dpi_ratio);
        closeBtn->move(w/2 - 50 * m_dpi_ratio, h - 76 * m_dpi_ratio);
    });
    chkBox->show();
    comntLbl->show();
    closeBtn->show();
}

void MainWindow::startUpdate()
{
    wstring tmp_path;
    if (m_package == L"msi") {
        wstring prodCode = NS_Utils::MsiProductCode(_T(REG_UNINST_KEY));
        if (prodCode.empty())
            prodCode = NS_Utils::MsiProductCode(_T(REG_GROUP_KEY));
        if (prodCode.empty()) {
            m_comntInfoLbl->setText(_TR(LABEL_ERR_PROD_CODE), true);
            createCloseAndBackButtons();
            return;
        }
        wstring packageName =  NS_Utils::MsiGetProperty(prodCode.c_str(), INSTALLPROPERTY_PACKAGENAME);
        if (packageName.empty()) {
            m_comntInfoLbl->setText(_TR(LABEL_ERR_PACK_NAME), true);
            createCloseAndBackButtons();
            return;
        }
        tmp_path = NS_File::toNativeSeparators(NS_File::tempPath() + _T("/") + packageName);
    } else {
        tmp_path = NS_File::toNativeSeparators(NS_File::generateTmpFileName(L"." + m_package));
    }

    CDownloader *dnl = startDownload(m_package == L"msi" ? L"msi" : L"iss", m_arch, tmp_path, [=]() {
            if (!NS_Utils::checkAndWaitForAppClosure(platformWindow())) {
                m_bar->setProgress(0);
                m_comntInfoLbl->setText(_TR(LABEL_ERR_CANCELLED), true);
                return;
            }
            m_bar->pulse(true);
            m_comntLbl->setText(_TR(LABEL_UPDATING));
            wstring args = L"/c call \"" + tmp_path;
            args += (m_package == L"msi") ? L"\" /qn /norestart" : L"\" /UPDATE /VERYSILENT /NOLAUNCH";
            m_future = std::async(std::launch::async, [=]() {
            DWORD status = NS_File::runProcess(L"cmd", args, true);
            UIThread::invoke(this, [=]() {
            if (status != 0) {
                DWORD err = GetLastError();
                m_bar->pulse(false);
                m_bar->setProgress(0);
                m_comntInfoLbl->setText(err == 0 ? _TR(LABEL_ERR_RUNNING) + wstring(L" Exit code: ") + to_wstring(status) : NS_Utils::GetLastErrorAsString(err), true);
            } else {
                if (m_checkState & ClrDataCheck) {
                    wstring dataPath = NS_File::appDataPath();
                    if (!dataPath.empty())
                        NS_File::removeDirRecursively(dataPath);
                }
                if (m_checkState & ClrStnCheck) {
                    wstring key(L"SOFTWARE\\");
                    key.append(_T(REG_GROUP_KEY));
                    SHDeleteKey(HKEY_CURRENT_USER, key.c_str());
                }
                m_bar->pulse(false);
                m_bar->setProgress(100);
                m_comntLbl->setText(_TR(LABEL_UPDATE_COMPL));
                m_versionLbl->setText(fillInstalledVerInfo());
                m_is_completed = true;
            }
            if (NS_File::fileExists(tmp_path))
                NS_File::removeFile(tmp_path);
            if (m_mode == Mode::Control)
                createCloseAndBackButtons();
            });
            });
        });

    m_cancelBtn->onClick([=]() {
        dnl->stop();
    });
}

// void MainWindow::startRepair()
// {
//     wstring tmp_path;
//     if (m_package == L"msi") {
//         wstring prodCode = NS_Utils::MsiProductCode(_T(REG_UNINST_KEY));
//         if (prodCode.empty())
//             prodCode = NS_Utils::MsiProductCode(_T(REG_GROUP_KEY));
//         if (prodCode.empty()) {
//             m_comntInfoLbl->setText(_TR(LABEL_ERR_PROD_CODE), true);
//             createCloseAndBackButtons();
//             return;
//         }
//         wstring packageName =  NS_Utils::MsiGetProperty(prodCode.c_str(), INSTALLPROPERTY_PACKAGENAME);
//         if (packageName.empty()) {
//             m_comntInfoLbl->setText(_TR(LABEL_ERR_PACK_NAME), true);
//             createCloseAndBackButtons();
//             return;
//         }
//         tmp_path = NS_File::toNativeSeparators(NS_File::tempPath() + _T("/") + packageName);
//     } else {
//         tmp_path = NS_File::toNativeSeparators(NS_File::generateTmpFileName(L"." + m_package));
//     }

//     wstring url = L"https://github.com/%1/%2/releases/download/%3/%4";
//     {
//         wstring url_filename = L"DesktopEditors_" + m_arch;
//         url_filename.append(L"." + m_package);

//         wstring url_ver = L"v" + m_ver;
//         size_t pos = url_ver.find_last_of(L'.');
//         if (pos != std::wstring::npos)
//             url_ver = url_ver.substr(0, pos);

//         NS_Utils::Replace(url, L"%1", _T(REG_GROUP_KEY));
//         NS_Utils::Replace(url, L"%2", _T(APP_NAME));
//         NS_Utils::Replace(url, L"%3", url_ver);
//         NS_Utils::Replace(url, L"%4", url_filename);
//     }

//     CDownloader *dnl = startDownload(m_package == L"msi" ? L"msi" : L"iss", m_arch, tmp_path, [=]() {
//             if (!NS_Utils::checkAndWaitForAppClosure(nativeWindowHandle())) {
//                 m_bar->setProgress(0);
//                 m_comntInfoLbl->setText(_TR(LABEL_ERR_CANCELLED), true);
//                 return;
//             }
//             m_bar->pulse(true);
//             wstring cmd = (m_package == L"msi") ? L"msiexec" : L"cmd",
//                 args = (m_package == L"msi") ? L"/fvamus \"" : L"/c \"";
//                 args += tmp_path;
//                 args += (m_package == L"msi") ? L"\" /qn" : L" /VERYSILENT\"";
//             m_future = std::async(std::launch::async, [=]() {
//             DWORD status = NS_File::runProcess(cmd, args, true);
//             UIThread::invoke(this, [=]() {
//             if (status != 0) {
//                 DWORD err = GetLastError();
//                 m_bar->pulse(false);
//                 m_bar->setProgress(0);
//                 m_comntInfoLbl->setText(err == 0 ? _TR(LABEL_ERR_RUNNING) + wstring(L" Exit code: ") + to_wstring(status) : NS_Utils::GetLastErrorAsString(err), true);
//             } else {
//                 if (m_checkState & ClrDataCheck) {
//                     wstring dataPath = NS_File::appDataPath();
//                     if (!dataPath.empty())
//                         NS_File::removeDirRecursively(dataPath);
//                 }
//                 if (m_checkState & ClrStnCheck) {
//                     wstring key(L"SOFTWARE\\");
//                     key.append(_T(REG_GROUP_KEY));
//                     SHDeleteKey(HKEY_CURRENT_USER, key.c_str());
//                 }
//                 m_bar->pulse(false);
//                 m_bar->setProgress(100);
//                 m_comntLbl->setText(_TR(LABEL_REPAIR_COMPL));
//                 m_is_completed = true;
//             }
//             if (NS_File::fileExists(tmp_path))
//                 NS_File::removeFile(tmp_path);
//             if (m_mode == Mode::Control)
//                 createCloseAndBackButtons();
//             });
//             });
//         });

//     m_cancelBtn->onClick([=]() {
//         dnl->stop();
//     });
// }

void MainWindow::startUninstall()
{
    m_cancelBtn->setDisabled(true);
    if (!NS_Utils::checkAndWaitForAppClosure(platformWindow())) {
        m_bar->setProgress(0);
        m_comntInfoLbl->setText(_TR(LABEL_ERR_CANCELLED), true);
        createCloseAndBackButtons();
        return;
    }
    m_bar->pulse(true);
    wstring args = L"/c \"" + m_uninst_cmd;
    args += (m_package == L"msi") ? L" /qn\"" : L" /VERYSILENT\"";
    m_future = std::async(std::launch::async, [=]() {
        DWORD status = NS_File::runProcess(L"cmd", args, true);
        UIThread::invoke(this, [=]() {
        if (status != 0) {
            m_bar->pulse(false);
            m_bar->setProgress(0);
            m_comntInfoLbl->setText(_TR(LABEL_ERR_UNINST));
            createCloseAndBackButtons();
        } else {
            if (m_checkState & ClrAllCheck) {
                wstring dataPath = NS_File::appDataPath();
                if (!dataPath.empty())
                    NS_File::removeDirRecursively(dataPath);

                wstring key(L"SOFTWARE\\");
                key.append(_T(REG_GROUP_KEY));
                SHDeleteKey(HKEY_CURRENT_USER, key.c_str());
            }
            m_bar->pulse(false);
            m_bar->setProgress(100);
            m_comntLbl->setText(_TR(LABEL_UNINST_COMPL));
            // m_is_completed = true;
            createCloseButton();
        }
        });
    });
}

void MainWindow::createSelectionPage()
{
    m_is_completed = false;
    /* Check box section*/
    UICheckBox *clrChkBox = new UICheckBox(m_cenPanel, _TR(CHECK_CLR_DATA));
    clrChkBox->setObjectGroupId(_T("Selector"));
    clrChkBox->setDisabled(!(m_checkState & UpdateRadio));
    clrChkBox->setChecked(m_checkState & ClrDataCheck);
    clrChkBox->adjustSizeBasedOnContent();
    clrChkBox->move(79 * m_dpi_ratio, 80 * m_dpi_ratio);
    clrChkBox->onClick([=]() {
        m_checkState = (m_checkState & ~ClrDataCheck) | (clrChkBox->isChecked() * ClrDataCheck);
    });

    UICheckBox *stnChkBox = new UICheckBox(m_cenPanel, _TR(CHECK_CLR_STNGS));
    stnChkBox->setObjectGroupId(_T("Selector"));
    stnChkBox->setDisabled(!(m_checkState & UpdateRadio));
    stnChkBox->setChecked(m_checkState & ClrStnCheck);
    stnChkBox->adjustSizeBasedOnContent();
    stnChkBox->move(79 * m_dpi_ratio, 112 * m_dpi_ratio);
    stnChkBox->onClick([stnChkBox, this]() {
        m_checkState = (m_checkState & ~ClrStnCheck) | (stnChkBox->isChecked() * ClrStnCheck);
    });

    UICheckBox *clrAllChkBox = new UICheckBox(m_cenPanel, _TR(CHECK_CLR_ALL));
    clrAllChkBox->setObjectGroupId(_T("Selector"));
    clrAllChkBox->setDisabled(!(m_checkState & UninstRadio));
    clrAllChkBox->setChecked(m_checkState & ClrAllCheck);
    clrAllChkBox->adjustSizeBasedOnContent();
    clrAllChkBox->move(79 * m_dpi_ratio, 182 * m_dpi_ratio);
    clrAllChkBox->onClick([clrAllChkBox, this]() {
        m_checkState = (m_checkState & ~ClrAllCheck) | (clrAllChkBox->isChecked() * ClrAllCheck);
    });

    /* Update radio button section*/
    m_updRadio = new UIRadioButton(m_cenPanel, _TR(RADIO_UPDATE));
    m_updRadio->setObjectGroupId(_T("Selector"));
    m_updRadio->setChecked(m_checkState & UpdateRadio);
    m_updRadio->adjustSizeBasedOnContent();
    m_updRadio->move(50 * m_dpi_ratio, 48 * m_dpi_ratio);
    m_updRadio->onClick([=]() {
        clrChkBox->setDisabled(false);
        stnChkBox->setDisabled(false);
        clrAllChkBox->setDisabled(true);
        // if (m_repRadio->isChecked())
        //     m_repRadio->setChecked(false);
        if (m_uninsRadio->isChecked())
            m_uninsRadio->setChecked(false);
        m_checkState = (m_checkState | UpdateRadio) & ~(RepairRadio | UninstRadio);
    });

    /* Repair radio button section*/
    // m_repRadio = new UIRadioButton(m_cenPanel, _TR(RADIO_REPAIR));
    // m_repRadio->setObjectGroupId(_T("Selector"));
    // m_repRadio->setChecked(m_checkState & RepairRadio);
    // m_repRadio->adjustSizeBasedOnContent();
    // m_repRadio->move(50 * m_dpi_ratio, 82 * m_dpi_ratio);
    // m_repRadio->onClick([=]() {
    //     clrChkBox->setDisabled(false);
    //     stnChkBox->setDisabled(false);
    //     clrAllChkBox->setDisabled(true);
    //     if (m_updRadio->isChecked())
    //         m_updRadio->setChecked(false);
    //     if (m_uninsRadio->isChecked())
    //         m_uninsRadio->setChecked(false);
    //     m_checkState = (m_checkState | RepairRadio) & ~(UninstRadio | UpdateRadio);
    // });

    /* Uninstall radio button section*/
    m_uninsRadio = new UIRadioButton(m_cenPanel, _TR(RADIO_UNINST));
    m_uninsRadio->setObjectGroupId(_T("Selector"));
    m_uninsRadio->setChecked(m_checkState & UninstRadio);
    m_uninsRadio->adjustSizeBasedOnContent();
    m_uninsRadio->move(50 * m_dpi_ratio, 150 * m_dpi_ratio);
    m_uninsRadio->onClick([=]() {
        clrChkBox->setDisabled(true);
        stnChkBox->setDisabled(true);
        clrAllChkBox->setDisabled(false);
        // if (m_repRadio->isChecked())
        //     m_repRadio->setChecked(false);
        if (m_updRadio->isChecked())
            m_updRadio->setChecked(false);
        m_checkState = (m_checkState | UninstRadio) & ~(UpdateRadio | RepairRadio);
    });

    /* Apply button section */
    UIButton *applyBtn = new UIButton(m_cenPanel);
    applyBtn->setObjectGroupId(_T("PushButton"));
    applyBtn->setText(_TR(BUTTON_APPLY));
    applyBtn->move(m_cenPanel->size().width - 112 * m_dpi_ratio, m_cenPanel->size().height - 40 * m_dpi_ratio);
    applyBtn->onClick([=]() {
        wstring msg = m_uninsRadio->isChecked() ? _TR(MSG_REMOVE) : /*m_repRadio->isChecked() ? _TR(MSG_REPAIR) :*/ _TR(MSG_UPDATE);
        NS_Utils::Replace(msg, L"%1", _T(WINDOW_NAME));
        if (IDOK == NS_Utils::ShowTaskDialog(platformWindow(), msg.c_str(), TD_WARNING_ICON)) {
            if (!NS_Utils::checkAndWaitForAppClosure(platformWindow()))
                return;
            m_cenPanel->disconnect(m_resize_conn);
            m_updRadio->close();
            // m_repRadio->close();
            clrChkBox->close();
            stnChkBox->close();
            m_uninsRadio->close();
            clrAllChkBox->close();
            applyBtn->close();
            msg = m_uninsRadio->isChecked() ? _TR(LABEL_UNINSTLING) : /*m_repRadio->isChecked() ? _TR(LABEL_REPAIRING) :*/ _TR(LABEL_DOWNLOAD);
            createProgressPage(msg);
            if (m_updRadio->isChecked() /*|| m_repRadio->isChecked()*/) {
                /* Check box section*/
                m_launchCheck = new UICheckBox(m_cenPanel, _TR(CHECK_LAUNCH));
                m_launchCheck->setObjectGroupId(_T("Selector"));
                m_launchCheck->setChecked(m_checkState & LaunchCheck);
                m_launchCheck->adjustSizeBasedOnContent();
                m_launchCheck->onClick([this]() {
                    m_checkState = (m_checkState & ~LaunchCheck) | (m_launchCheck->isChecked() * LaunchCheck);
                });
            }
            if (m_uninsRadio->isChecked())
                startUninstall();
            else
            // if (m_repRadio->isChecked())
            //     startRepair();
            // else
                startUpdate();
        }
    });
    m_resize_conn = m_cenPanel->onResize([this, clrAllChkBox, stnChkBox, clrChkBox, applyBtn](int w, int h) {
        m_updRadio->move(50 * m_dpi_ratio, 48 * m_dpi_ratio);
        // m_repRadio->move(50 * m_dpi_ratio, 82 * m_dpi_ratio);
        m_uninsRadio->move(50 * m_dpi_ratio, 150 * m_dpi_ratio);
        clrAllChkBox->move(79 * m_dpi_ratio, 182 * m_dpi_ratio);
        stnChkBox->move(79 * m_dpi_ratio, 112 * m_dpi_ratio);
        clrChkBox->move(79 * m_dpi_ratio, 80 * m_dpi_ratio);
        applyBtn->move(w - 112 * m_dpi_ratio, h - 40 * m_dpi_ratio);
    });
    m_updRadio->show();
    // m_repRadio->show();
    clrChkBox->show();
    stnChkBox->show();
    m_uninsRadio->show();
    clrAllChkBox->show();
    applyBtn->show();
}

void MainWindow::createProgressPage(const std::wstring &text)
{
    m_comntLbl = new UILabel(m_cenPanel);
    m_comntLbl->setObjectGroupId(_T("ControlLabel"));
    m_comntLbl->setText(text);
    m_comntLbl->metrics()->setMetrics(Metrics::TextMarginLeft, 12);
    m_comntLbl->setGeometry(30 * m_dpi_ratio, 50 * m_dpi_ratio, size().width - 30 * m_dpi_ratio, 24 * m_dpi_ratio);

    m_comntInfoLbl = new UILabel(m_cenPanel);
    m_comntInfoLbl->setObjectGroupId(_T("ControlLabel"));
    m_comntInfoLbl->metrics()->setMetrics(Metrics::TextMarginLeft, 12);
    m_comntInfoLbl->setGeometry(30 * m_dpi_ratio, 92 * m_dpi_ratio, m_cenPanel->size().width - 30 * m_dpi_ratio, 40 * m_dpi_ratio);

    /* Progress section */
    m_bar = new UIProgressBar(m_cenPanel);
    m_bar->setObjectGroupId(_T("ProgressBar"));
    m_bar->setBaseSize(250, 5);
    m_bar->move(42 * m_dpi_ratio, 80 * m_dpi_ratio);

    m_cancelBtn = new UIButton(m_cenPanel);
    m_cancelBtn->setObjectGroupId(_T("PushButton"));
    m_cancelBtn->setText(_TR(BUTTON_CANCEL));
    m_cancelBtn->move(m_cenPanel->size().width - 112 * m_dpi_ratio, m_cenPanel->size().height - 40 * m_dpi_ratio);
    m_resize_conn = m_cenPanel->onResize([this](int w, int h) {
        m_comntLbl->setGeometry(30 * m_dpi_ratio, 50 * m_dpi_ratio, size().width - 30 * m_dpi_ratio, 24 * m_dpi_ratio);
        m_comntInfoLbl->setGeometry(30 * m_dpi_ratio, 92 * m_dpi_ratio, m_cenPanel->size().width - 30 * m_dpi_ratio, 40 * m_dpi_ratio);
        m_bar->move(42 * m_dpi_ratio, 80 * m_dpi_ratio);
        m_cancelBtn->move(w - 112 * m_dpi_ratio, h - 40 * m_dpi_ratio);
    });

    m_comntLbl->show();
    m_comntInfoLbl->show();
    m_bar->show();
    m_cancelBtn->show();
}

void MainWindow::createCloseButton()
{
    UIThread::invoke(this, [=]() {
        m_cenPanel->disconnect(m_resize_conn);
        m_cancelBtn->close();
        UIButton *closeBtn = new UIButton(m_cenPanel);
        closeBtn->setObjectGroupId(_T("PushButton"));
        closeBtn->setText(_TR(BUTTON_CLOSE));
        closeBtn->move(m_cenPanel->size().width - 112 * m_dpi_ratio, m_cenPanel->size().height - 40 * m_dpi_ratio);
        closeBtn->onClick([=]() {
            m_cenPanel->disconnect(m_resize_conn);
            close();
        });
        m_resize_conn = m_cenPanel->onResize([this, closeBtn](int w, int h) {
            closeBtn->move(w - 112 * m_dpi_ratio, h - 40 * m_dpi_ratio);
            if (m_comntLbl)
                m_comntLbl->setGeometry(30 * m_dpi_ratio, 50 * m_dpi_ratio, size().width - 30 * m_dpi_ratio, 24 * m_dpi_ratio);
            if (m_comntInfoLbl)
                m_comntInfoLbl->setGeometry(30 * m_dpi_ratio, 92 * m_dpi_ratio, m_cenPanel->size().width - 30 * m_dpi_ratio, 40 * m_dpi_ratio);
            if (m_bar)
                m_bar->move(42 * m_dpi_ratio, 80 * m_dpi_ratio);
        });
        closeBtn->show();
    });
}

void MainWindow::createCloseAndBackButtons()
{
    UIThread::invoke(this, [=]() {
        m_cenPanel->disconnect(m_resize_conn);
        m_cancelBtn->close();

        if (m_launchCheck) {
            if (m_is_completed) {
                m_comntInfoLbl->hide();
                m_launchCheck->move(42 * m_dpi_ratio, 100 * m_dpi_ratio);
                m_launchCheck->show();
            } else {
                m_launchCheck->close();
                m_launchCheck = nullptr;
            }
        }

        UIButton *closeBtn = new UIButton(m_cenPanel);
        closeBtn->setObjectGroupId(_T("PushButton"));
        closeBtn->setText(_TR(BUTTON_CLOSE));
        closeBtn->move(m_cenPanel->size().width - 112 * m_dpi_ratio, m_cenPanel->size().height - 40 * m_dpi_ratio);
        closeBtn->onClick([=]() {
            m_cenPanel->disconnect(m_resize_conn);
            if (m_launchCheck && m_is_completed && (m_checkState & LaunchCheck)) {
                wstring app_path;
                if (NS_Utils::IsAppInstalled(app_path))
                    NS_File::runProcess(L"explorer.exe", app_path + _T(APP_LAUNCH_NAME), false);
            }
            close();
        });

        UIButton *backBtn = new UIButton(m_cenPanel);
        backBtn->setObjectGroupId(_T("PushButton"));
        backBtn->setText(_TR(BUTTON_BACK));
        backBtn->move(m_cenPanel->size().width - 218 * m_dpi_ratio, m_cenPanel->size().height - 40 * m_dpi_ratio);
        backBtn->onClick([=]() {
            m_cenPanel->disconnect(m_resize_conn);
            if (m_launchCheck) {
                m_launchCheck->close();
                m_launchCheck = nullptr;
            }
            m_comntLbl->close();
            m_comntInfoLbl->close();
            m_bar->close();
            closeBtn->close();
            backBtn->close();
            createSelectionPage();
        });

        m_resize_conn = m_cenPanel->onResize([this, closeBtn, backBtn](int w, int h) {
            closeBtn->move(w - 112 * m_dpi_ratio, h - 40 * m_dpi_ratio);
            backBtn->move(w - 218 * m_dpi_ratio, h - 40 * m_dpi_ratio);
            if (m_comntLbl)
                m_comntLbl->setGeometry(30 * m_dpi_ratio, 50 * m_dpi_ratio, size().width - 30 * m_dpi_ratio, 24 * m_dpi_ratio);
            if (m_comntInfoLbl)
                m_comntInfoLbl->setGeometry(30 * m_dpi_ratio, 92 * m_dpi_ratio, m_cenPanel->size().width - 30 * m_dpi_ratio, 40 * m_dpi_ratio);
            if (m_bar)
                m_bar->move(42 * m_dpi_ratio, 80 * m_dpi_ratio);
            if (m_launchCheck)
                m_launchCheck->move(42 * m_dpi_ratio, 100 * m_dpi_ratio);
        });

        closeBtn->show();
        backBtn->show();
    });
}

wstring MainWindow::fillInstalledVerInfo()
{
    wstring text = _TR(LABEL_VERSION), dispName;
    NS_Utils::InstalledVerInfo(L"DisplayName", dispName, m_arch);
    NS_Utils::InstalledVerInfo(L"DisplayVersion", m_ver, m_arch);
    if (m_ver.empty())
        m_ver = _TR(LABEL_UNKN_VER);

    NS_Utils::InstalledVerInfo(L"UninstallString", m_uninst_cmd, m_arch);
    m_package = (m_uninst_cmd.find(L"msiexec") != std::wstring::npos) ? L"msi" : (m_uninst_cmd.find(L".exe") != std::wstring::npos) ? L"exe" : _TR(LABEL_UNKN_PACK);

    if (!dispName.empty()) {
        NS_Utils::Replace(text, L"%1 %2 (%3 %4)", displayNameAddInfo(dispName, m_package));
    } else {
        NS_Utils::Replace(text, L"%1", _T(WINDOW_NAME));
        NS_Utils::Replace(text, L"%2", m_ver);
        NS_Utils::Replace(text, L"%3", m_arch);
        NS_Utils::Replace(text, L"%4", m_package);
    }
    return text;
}

CDownloader* MainWindow::startDownload(const std::wstring &install_type, const std::wstring &arch, const std::wstring &path, const std::function<void()> &onComplete)
{
    wstring appcast_url = NS_Utils::cmdArgContains(_T("--appcast-dev-channel")) ? _T(URL_INSTALL_DEV) : _T(URL_INSTALL);
    wstring tmp_path = NS_File::toNativeSeparators(NS_File::generateTmpFileName(L".json"));
    NS_Logger::WriteLog(_T("Appcast URL:\n") + appcast_url);
    CDownloader *dnl = new CDownloader();
    dnl->onComplete([=](ulong error) {
        if (error == ERROR_SUCCESS) {
            list<tstring> lst;
            if (NS_File::readFile(tmp_path, lst)) {
                tstring json = std::accumulate(lst.begin(), lst.end(), tstring());
                JsonDocument doc(json);
                JsonObject root = doc.object();

                // tstring version = root.value(_T("version")).toTString();
                JsonObject package = root.value(_T("package")).toObject();
#ifdef _WIN32
                JsonObject win = package.value(arch == _T("x64") ? _T("win_64") : _T("win_32")).toObject();
#else
                JsonObject win = package.value(_T("linux_64")).toObject();
#endif
                JsonObject package_type = win.value(install_type).toObject();
                tstring url = package_type.value(_T("url")).toTString();
                tstring url2 = package_type.value(_T("url2")).toTString();
                NS_Logger::WriteLog(_T("Primary package URL:\n") + url + _T("\n\nSecondary package URL:\n") + url2);
                if ((url.empty() || !dnl->isUrlAccessible(url)) && !url2.empty())
                    url = url2;
                NS_Logger::WriteLog(_T("Download from URL:\n") + url);
                tstring hash = package_type.value(_T("md5")).toTString();
                std::transform(hash.begin(), hash.end(), hash.begin(), ::tolower);
                NS_File::removeFile(tmp_path);

                UIThread::invoke(this, [=]() {
                    dnl->stop();
                    dnl->onProgress([=](int percent) {
                        m_bar->setProgress(percent);
                    });
                    dnl->onComplete([=](ulong error) {
                        UIThread::invoke(this, [=]() {
                        if (m_mode == Mode::Control)
                            m_cancelBtn->setDisabled(true);
                        if (error == ERROR_SUCCESS) {
                            tstring fhash = NS_File::getFileHash(path);
                            if (!fhash.empty() && fhash == hash && NS_File::verifyEmbeddedSignature(path)) {
                                onComplete();
                            } else {
                                m_bar->setProgress(0);
                                m_comntInfoLbl->setText(_TR(LABEL_NO_VER_AVAIL), true);
                                if (m_mode == Mode::Control)
                                    createCloseAndBackButtons();
                            }
                        } else
                        if (error == ERROR_CANCELLED) {
                            m_comntInfoLbl->setText(_TR(LABEL_ERR_CANCELLED), true);
                            if (m_mode == Mode::Control)
                                createCloseAndBackButtons();
                        } else {
                            m_comntInfoLbl->setText(NS_Utils::GetLastErrorAsString(error), true);
                            if (m_mode == Mode::Control)
                                createCloseAndBackButtons();
                        }
                        });
                    });
                    dnl->downloadFile(url, path);
                });
            } else {
                UIThread::invoke(this, [=]() {
                m_comntInfoLbl->setText(NS_Utils::GetLastErrorAsString(), true);
                NS_File::removeFile(tmp_path);
                if (m_mode == Mode::Control)
                    createCloseAndBackButtons();
                });
            }
        } else
        if (error == ERROR_CANCELLED) {
            UIThread::invoke(this, [=]() {
            m_comntInfoLbl->setText(_TR(LABEL_ERR_CANCELLED), true);
            if (m_mode == Mode::Control)
                createCloseAndBackButtons();
            });
        } else {
            UIThread::invoke(this, [=]() {
            m_comntInfoLbl->setText(NS_Utils::GetLastErrorAsString(error), true);
            if (m_mode == Mode::Control)
                createCloseAndBackButtons();
            });
        }
    });
    dnl->downloadFile(appcast_url, tmp_path);
    onAboutToDestroy([=]() {
        delete dnl;
    });
    return dnl;
}
