#include "uipopupmessage.h"
#include "uidrawningengine.h"
#include "uieventloop.h"
#include "uimetrics.h"
#include "uiboxlayout.h"
#include "uibutton.h"
#include "uilabel.h"
#include "uistyle.h"


UIPopupMessage::UIPopupMessage(UIWidget *parent, const Rect &rc) :
    UIAbstractPopup(parent, rc),
    m_loop(new UIEventLoop),
    m_selected(ButtonRole::None)
{
#ifdef _WIN32
#else
    gtk_window_set_modal(GTK_WINDOW(m_hWindow), TRUE);
#endif

    UIBoxLayout *lvlut = new UIBoxLayout(UIBoxLayout::Vertical, UIBoxLayout::AlignHCenter | UIBoxLayout::AlignVCenter);
    lvlut->setContentMargins(17, 17, 17, 17);
    lvlut->setSpacing(0);
    setLayout(lvlut);

    UIWidget *top = new UIWidget(this);
    top->setObjectGroupId(_T("PopupMsgTopWidget"));
    top->setCorners(UIDrawingEngine::CornerLTop | UIDrawingEngine::CornerRTop);
    top->setSizePolicy(SizePolicy::HSizeBehavior, SizePolicy::Expanding);
    top->setSizePolicy(SizePolicy::VSizeBehavior, SizePolicy::Expanding);
    top->metrics()->setMetrics(Metrics::BorderWidth, 0);
    top->metrics()->setMetrics(Metrics::BorderRadius, 7);
    lvlut->addWidget(top);

    UIWidget *sep = new UIWidget(this);
    sep->setObjectGroupId(_T("PopupMsgSeparator"));
    sep->setSizePolicy(SizePolicy::HSizeBehavior, SizePolicy::Expanding);
    sep->setSizePolicy(SizePolicy::VSizeBehavior, SizePolicy::Fixed);
    sep->metrics()->setMetrics(Metrics::BorderWidth, 0);
    sep->metrics()->setMetrics(Metrics::BorderRadius, 0);
    sep->setBaseSize(50, 1);
    lvlut->addWidget(sep);

    buttonsWidget = new UIWidget(this);
    buttonsWidget->setObjectGroupId(_T("PopupMsgBottomWidget"));
    buttonsWidget->setCorners(UIDrawingEngine::CornerLBottom | UIDrawingEngine::CornerRBottom);
    buttonsWidget->setSizePolicy(SizePolicy::HSizeBehavior, SizePolicy::Expanding);
    buttonsWidget->setSizePolicy(SizePolicy::VSizeBehavior, SizePolicy::Fixed);
    buttonsWidget->metrics()->setMetrics(Metrics::BorderWidth, 0);
    buttonsWidget->metrics()->setMetrics(Metrics::BorderRadius, 7);
    buttonsWidget->setBaseSize(50, 80);
    lvlut->addWidget(buttonsWidget);

    UIBoxLayout *labelsLayout = new UIBoxLayout(UIBoxLayout::Vertical, UIBoxLayout::AlignHCenter | UIBoxLayout::AlignVCenter);
    labelsLayout->setContentMargins(21, 23, 21, 12);
    labelsLayout->setSpacing(0);
    top->setLayout(labelsLayout);

    m_labelCaption = new UILabel(top);
    m_labelCaption->setObjectGroupId(_T("PopupMsgLabel"));
    m_labelCaption->setFont(_T("Arial"), 14);
    m_labelCaption->metrics()->setMetrics(Metrics::TextAlignment, Metrics::AlignHLeft | Metrics::AlignVCenter);
    labelsLayout->addWidget(m_labelCaption);

    m_labelDescription = new UILabel(top);
    m_labelDescription->setObjectGroupId(_T("PopupMsgLabel"));
    m_labelDescription->setFont(_T("Arial"), 9.5);
    m_labelDescription->metrics()->setMetrics(Metrics::TextAlignment, Metrics::AlignHLeft | Metrics::AlignVCenter);
    labelsLayout->addWidget(m_labelDescription);

    m_buttonsLayout = new UIBoxLayout(UIBoxLayout::Horizontal, UIBoxLayout::AlignHCenter | UIBoxLayout::AlignVCenter);
    m_buttonsLayout->setContentMargins(20, 0, 20, 0);
    m_buttonsLayout->setSpacing(0);
    buttonsWidget->setLayout(m_buttonsLayout);
}

UIPopupMessage::~UIPopupMessage()
{
    delete m_loop; m_loop = nullptr;
}

void UIPopupMessage::setCaptionText(const tstring &text)
{
    m_labelCaption->setText(text);
}

void UIPopupMessage::setDescriptionText(const tstring &text)
{
    m_labelDescription->setText(text);
}

void UIPopupMessage::addButton(const tstring &text, ButtonRole role)
{
    UIButton *btn = new UIButton(buttonsWidget);
    btn->setObjectGroupId(_T("PopupMsgButton"));
    btn->setText(text);
    m_buttonsLayout->addWidget(btn);
    btn->onClick([=]() {
        m_selected = role;
        m_loop->exit();
    });
}

int UIPopupMessage::runDialog()
{
    showAll();
    m_loop->exec();
#ifdef _WIN32
    if (UIWidget *parent = parentWidget())
        EnableWindow(parent->platformWindow(), TRUE);
#endif
    hide();
    return m_selected;
}

#ifdef _WIN32
bool UIPopupMessage::event(UINT msg, WPARAM wParam, LPARAM lParam, LRESULT *result)
{
    switch (msg) {
    case WM_SHOWWINDOW: {
        if (wParam) {
            if (UIWidget *parent = parentWidget())
                EnableWindow(parent->platformWindow(), FALSE);
        }
        break;
    }

    case WM_CLOSE: {
        if (UIWidget *parent = parentWidget())
            EnableWindow(parent->platformWindow(), TRUE);
        if (m_loop->isRunning()) {
            m_selected = ButtonRole::Close;
            m_loop->exit();
            *result = TRUE;
            return true;
        }
        break;
    }

    default:
        break;
    }
    return UIAbstractPopup::event(msg, wParam, lParam, result);
}
#else
bool UIPopupMessage::event(uint ev_type, void *param)
{
    switch (ev_type) {
    case GDK_DELETE: {
        if (m_loop->isRunning()) {
            m_selected = ButtonRole::Close;
            m_loop->exit();
            return true;
        }
        break;
    }

    default:
        break;
    }
    return UIAbstractPopup::event(ev_type, param);
}
#endif
