#ifndef UIOBJECT_H
#define UIOBJECT_H

#include "uidefines.h"


class DECL_VISUALUI UIObject
{
public:
    enum ObjectType : unsigned char {
        ApplicationType,
        WindowType,
        DialogType,
        WidgetType,
        PopupType
    };

    explicit UIObject(ObjectType type, UIObject *parent = nullptr);
    virtual ~UIObject();

    void setParent(UIObject*) noexcept;
    void setObjectName(const tstring&);
    virtual void setObjectGroupId(const tstring &id);
    UIObject *parent() const noexcept;
    ObjectType objectType() const noexcept;
    tstring objectName() const noexcept;
    tstring objectGroupId() const noexcept;
    virtual void disconnect(int);

protected:
    static int m_connectionId;

private:
    UIObject();

    UIObject  *m_parent;
    ObjectType m_objectType;
    tstring    m_objectName;
    tstring    m_groupId;
};

#endif // UIOBJECT_H
