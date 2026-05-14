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

#ifndef UIXMLDOCUMENT_H
#define UIXMLDOCUMENT_H

#include "uidefines.h"
#include <vector>


class XmlDocument;
class DECL_VISUALUI XmlNode
{
public:
    XmlNode();
    XmlNode(const XmlNode& other);
    XmlNode(XmlNode&& other) noexcept;
    ~XmlNode();

    XmlNode& operator=(const XmlNode& other);
    XmlNode& operator=(XmlNode&& other) noexcept;

    bool isValid() const noexcept;
    tstring getTagName() const;
    tstring getText() const;
    tstring getAttributeValue(const tstring &name) const;
    std::vector<std::pair<tstring, tstring>> getAttributes() const;
    std::vector<XmlNode> getChildren() const;

    bool setText(const tstring &text);
    bool setAttribute(const tstring &name, const tstring &value);
    XmlNode appendChild(const tstring &tagName);

private:
    friend XmlDocument;
    class XmlNodePrivate;
    XmlNodePrivate *pimpl;
};


class DECL_VISUALUI XmlDocument
{
public:

    XmlDocument();
    ~XmlDocument();

    bool loadFromFile(const tstring &fileName);
    bool loadFromXml(const tstring &xml);
    XmlNode createDocument(const tstring &rootName);
    XmlNode createElement(XmlNode &parent, const tstring &tagName);
    XmlNode root() const;
    bool saveToFile(const tstring &fileName) const;
    tstring toString() const;

private:
    class XmlDocumentPrivate;
    XmlDocumentPrivate *pimpl;
};

#endif // UIXMLDOCUMENT_H
