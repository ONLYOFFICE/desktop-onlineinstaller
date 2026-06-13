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

#include "uiconhandler.h"
#include "uimetrics.h"
#include "uiwidget.h"
#ifdef _WIN32
# include "uiutils.h"
#else
# include <librsvg-2.0/librsvg/rsvg.h>
#endif


UIconHandler::UIconHandler(UIWidget *owner) :
    m_hBmp(nullptr),
#ifdef _WIN32
    m_hIcon(nullptr),
    m_hEmf(nullptr),
#else
    m_hSvg(nullptr),
#endif
    m_owner(owner)
{

}

UIconHandler::~UIconHandler()
{
#ifdef _WIN32
    if (m_hIcon) {
        DestroyIcon(m_hIcon);
        m_hIcon = nullptr;
    }
    if (m_hEmf) {
        delete m_hEmf;
        m_hEmf = nullptr;
    }
    if (m_hBmp) {
        delete m_hBmp;
        m_hBmp = nullptr;
    }
#else
    if (m_hSvg) {
        g_object_unref(m_hSvg);
        m_hSvg = nullptr;
    }
    if (m_hBmp) {
        g_object_unref(m_hBmp);
        m_hBmp = nullptr;
    }
#endif
}

void UIconHandler::setIcon(const tstring &path, int w, int h)
{
    m_owner->metrics()->setMetrics(Metrics::IconWidth, w);
    m_owner->metrics()->setMetrics(Metrics::IconHeight, h);
#ifdef _WIN32
    if (m_hIcon) {
        DestroyIcon(m_hIcon);
        m_hIcon = nullptr;
    }
    m_hIcon = (HICON)LoadImage(NULL, path.c_str(), IMAGE_ICON, w, h, LR_LOADFROMFILE | LR_DEFAULTCOLOR | LR_SHARED);
#else
    if (m_hBmp) {
        g_object_unref(m_hBmp);
        m_hBmp = nullptr;
    }
    m_hBmp = gdk_pixbuf_new_from_file_at_scale(path.c_str(), w, h, FALSE, NULL);
#endif
    m_owner->update();
}

#ifdef _WIN32
void UIconHandler::setIcon(int id, int w, int h)
{
    if (m_hIcon) {
        DestroyIcon(m_hIcon);
        m_hIcon = nullptr;
    }
    m_owner->metrics()->setMetrics(Metrics::IconWidth, w);
    m_owner->metrics()->setMetrics(Metrics::IconHeight, h);
    HMODULE hInst = GetModuleHandle(NULL);
    m_hIcon = (HICON)LoadImage(hInst, MAKEINTRESOURCE(id), IMAGE_ICON, w, h, LR_COPYFROMRESOURCE | LR_DEFAULTCOLOR | LR_SHARED);
    m_owner->update();
}

void UIconHandler::setEMFIcon(const std::wstring &path, int w, int h)
{
    if (m_hEmf) {
        delete m_hEmf;
        m_hEmf = nullptr;
    }
    m_owner->metrics()->setMetrics(Metrics::IconWidth, w);
    m_owner->metrics()->setMetrics(Metrics::IconHeight, h);
    m_hEmf = new Gdiplus::Metafile(path.c_str());
    m_owner->update();
}

void UIconHandler::setEMFIcon(int id, int w, int h)
{
    if (m_hEmf) {
        delete m_hEmf;
        m_hEmf = nullptr;
    }
    m_owner->metrics()->setMetrics(Metrics::IconWidth, w);
    m_owner->metrics()->setMetrics(Metrics::IconHeight, h);
    UIUtils::loadEmfResource(m_hEmf, id, RT_RCDATA);
    m_owner->update();
}

void UIconHandler::setImage(int id, int w, int h)
{
    if (m_hBmp) {
        delete m_hBmp, m_hBmp = nullptr;
    }
    m_owner->metrics()->setMetrics(Metrics::IconWidth, w);
    m_owner->metrics()->setMetrics(Metrics::IconHeight, h);
    UIUtils::loadImageResource(m_hBmp, id, L"PNG");
    m_owner->update();
}
#else
void UIconHandler::setIcon(const char *id, int w, int h)
{
    m_owner->metrics()->setMetrics(Metrics::IconWidth, w);
    m_owner->metrics()->setMetrics(Metrics::IconHeight, h);
    if (m_hBmp) {
        g_object_unref(m_hBmp);
        m_hBmp = nullptr;
    }
    m_hBmp = gdk_pixbuf_new_from_resource_at_scale(id, w, h, FALSE, NULL);
    m_owner->update();
}

void UIconHandler::setSVGIcon(const tstring &path, int w, int h)
{
    if (m_hSvg) {
        g_object_unref(m_hSvg);
        m_hSvg = nullptr;
    }
    m_owner->metrics()->setMetrics(Metrics::IconWidth, w);
    m_owner->metrics()->setMetrics(Metrics::IconHeight, h);
    m_hSvg = rsvg_handle_new_from_file(path.c_str(), NULL);
    m_owner->update();
}

void UIconHandler::setSVGIcon(const char *id, int w, int h)
{
    if (m_hSvg) {
        g_object_unref(m_hSvg);
        m_hSvg = nullptr;
    }
    m_owner->metrics()->setMetrics(Metrics::IconWidth, w);
    m_owner->metrics()->setMetrics(Metrics::IconHeight, h);
    GBytes *bytes = g_resources_lookup_data(id, G_RESOURCE_LOOKUP_FLAGS_NONE, NULL);
    if (bytes) {
        gsize size = 0;
        gconstpointer data = g_bytes_get_data(bytes, &size);
        if (data && size > 0) {
            m_hSvg = rsvg_handle_new_from_data((const guint8*)data, size, NULL);
        }
        g_bytes_unref(bytes);
    }
    m_owner->update();
}

void UIconHandler::setImage(const char *id, int w, int h)
{
    setIcon(id, w, h);
}
#endif

void UIconHandler::setImage(const tstring &path, int w, int h)
{
#ifdef _WIN32
    m_owner->metrics()->setMetrics(Metrics::IconWidth, w);
    m_owner->metrics()->setMetrics(Metrics::IconHeight, h);
    if (m_hBmp) {
        delete m_hBmp, m_hBmp = nullptr;
    }
    m_hBmp = new Gdiplus::Bitmap(path.c_str());
    m_owner->update();
#else
    setIcon(path, w, h);
#endif
}

void UIconHandler::setPixmap(const UIPixmap &pixmap)
{
#ifdef _WIN32
    if (m_hBmp) {
        delete m_hBmp; m_hBmp = nullptr;
    }
#else
    if (m_hBmp) {
        g_object_unref(m_hBmp);
        m_hBmp = nullptr;
    }
#endif
    if (pixmap.isValid()) {
        Size sz = pixmap.imageSize();
#ifdef _WIN32
        Gdiplus::Bitmap *hBmp = pixmap.m_hBmp;
        m_hBmp = hBmp->Clone(0, 0, hBmp->GetWidth(), hBmp->GetHeight(), hBmp->GetPixelFormat());
#else
        m_hBmp = gdk_pixbuf_copy(pixmap.m_hBmp);
#endif
        setIconSize(sz.width, sz.height);
    }
}

void UIconHandler::setPixmap(UIPixmap &&pixmap)
{
#ifdef _WIN32
    if (m_hBmp) {
        delete m_hBmp; m_hBmp = nullptr;
    }
#else
    if (m_hBmp) {
        g_object_unref(m_hBmp);
        m_hBmp = nullptr;
    }
#endif
    if (pixmap.isValid()) {
        Size sz = pixmap.imageSize();
#ifdef _WIN32
        m_hBmp = pixmap.m_hBmp;
        pixmap.m_hBmp = nullptr;
#else
        m_hBmp = pixmap.m_hBmp;
        pixmap.m_hBmp = nullptr;
#endif
        setIconSize(sz.width, sz.height);
    }
}

void UIconHandler::setIconSize(int w, int h)
{
    m_owner->metrics()->setMetrics(Metrics::IconWidth, w);
    m_owner->metrics()->setMetrics(Metrics::IconHeight, h);
    m_owner->update();
}
