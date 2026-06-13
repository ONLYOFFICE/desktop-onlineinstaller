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

#include "cdownloader.h"
#include "httptransaction_p.h"
#include <future>


class CDownloaderPrivate
{
public:
    CDownloaderPrivate()
    {}
    ~CDownloaderPrivate()
    {}

    FnVoidUlUl  m_query_callback = nullptr;
    FnVoidUlStr m_complete_callback = nullptr;
    FnVoidInt   m_progress_callback = nullptr;

    int m_prev_percent = -1;

    std::future<void> m_future;
    std::atomic_bool  m_run{true},
                      m_lock{false};
};


CDownloader::CDownloader() :
    pimpl(new CDownloaderPrivate)
{}

CDownloader::~CDownloader()
{
    pimpl->m_run = false;
    if (pimpl->m_future.valid())
        pimpl->m_future.wait();

    delete pimpl; pimpl = nullptr;
}

bool CDownloader::isUrlAccessible(const wstring &url)
{
    if (url.empty())
        return false;

    HTTPTransaction tsn(url);
    tsn.setOption(HTTPTransaction::HeadOnly);
    tsn.setOption(HTTPTransaction::FollowRedirects);
    if (tsn.perform() != ERROR_SUCCESS)
        return false;

    return true;
}

void CDownloader::queryContentLenght(const wstring &url)
{
    if (url.empty())
    {
        if (pimpl->m_query_callback)
            pimpl->m_query_callback(ERROR_WINHTTP_INVALID_URL, 0);
        return;
    }

    bool expected = false;
    if (!pimpl->m_lock.compare_exchange_strong(expected, true))
    {
        if (pimpl->m_query_callback)
            pimpl->m_query_callback(ERROR_OPERATION_ABORTED, 0);
        return;
    }

    pimpl->m_future = std::async(std::launch::async, [=]() {
        ULONGLONG dwFileSize = 0;

        HTTPTransaction tsn(url);
        tsn.setOption(HTTPTransaction::HeadOnly);
        tsn.setOption(HTTPTransaction::FollowRedirects);
        DWORD res = tsn.perform();
        if (res == ERROR_SUCCESS)
            res = tsn.getContentLength(dwFileSize);

        if (pimpl->m_query_callback)
            pimpl->m_query_callback(res, (uint64_t)dwFileSize);

        pimpl->m_lock = false;
    });
}

void CDownloader::downloadFile(const std::wstring &url, const std::wstring &filePath)
{
    if (url.empty())
    {
        if (pimpl->m_complete_callback)
            pimpl->m_complete_callback(ERROR_WINHTTP_INVALID_URL, std::wstring());
        return;
    }

    if (filePath.empty())
    {
        if (pimpl->m_complete_callback)
            pimpl->m_complete_callback(ERROR_INVALID_NAME, std::wstring());
        return;
    }

    bool expected = false;
    if (!pimpl->m_lock.compare_exchange_strong(expected, true))
    {
        if (pimpl->m_complete_callback)
            pimpl->m_complete_callback(ERROR_OPERATION_ABORTED, std::wstring());
        return;
    }

    pimpl->m_run = true;

    pimpl->m_future = std::async(std::launch::async, [=]() {
        pimpl->m_prev_percent = -1;
        DWORD res = ERROR_SUCCESS;

        HANDLE hFile = CreateFile(filePath.c_str(), GENERIC_WRITE | DELETE, 0, NULL,
                                  CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile != INVALID_HANDLE_VALUE)
        {
            HTTPTransaction tsn(url);
            tsn.setFileHandle(hFile);
            tsn.setProgressCallback(
                +[](void* pData, ULONGLONG dltotal, ULONGLONG dlnow) -> DWORD
                {
                    CDownloaderPrivate *pimpl = (CDownloaderPrivate*)pData;
                    if (dltotal > 0 && pimpl->m_progress_callback)
                    {
                        int percent = static_cast<int>((100.0 * dlnow) / dltotal);
                        if (percent != pimpl->m_prev_percent)
                        {
                            pimpl->m_progress_callback(percent);
                            pimpl->m_prev_percent = percent;
                        }
                    }

                    if (pimpl->m_run == false)
                        return ERROR_CANCELLED;

                    return ERROR_SUCCESS;
                });
            tsn.setProgressData(pimpl);
            tsn.setOption(HTTPTransaction::Writedata);
            tsn.setOption(HTTPTransaction::FollowRedirects);
            res = tsn.perform();

            if (res != ERROR_SUCCESS)
            {
                FILE_DISPOSITION_INFO fdi = { TRUE };
                SetFileInformationByHandle(hFile, FileDispositionInfo, &fdi, sizeof(fdi));
            }
            CloseHandle(hFile);

        } else res = GetLastError();

        if (pimpl->m_complete_callback)
            pimpl->m_complete_callback(res, filePath);

        pimpl->m_lock = false;
    });
}

void CDownloader::stop()
{
    pimpl->m_run = false;
    if (pimpl->m_future.valid())
        pimpl->m_future.wait();
}

void CDownloader::onQueryResponse(FnVoidUlUl callback)
{
    pimpl->m_query_callback = callback;
}

void CDownloader::onComplete(FnVoidUlStr callback)
{
    pimpl->m_complete_callback = callback;
}

void CDownloader::onProgress(FnVoidInt callback)
{
    pimpl->m_progress_callback = callback;
}
