#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "uiwindow.h"
#include <future>


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
    void initControlMode(const std::wstring &arch);

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
    std::wstring fillInstalledVerInfo();
    CDownloader* startDownload(const std::wstring &install_type, const std::wstring &arch, const std::wstring &path, const std::function<void()> &onComplete);
    template<typename Fn, typename... Args>
    void invokeMethod(Fn&& fn, Args&&... args);

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
