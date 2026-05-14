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
