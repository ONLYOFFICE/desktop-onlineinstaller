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
