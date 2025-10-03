#ifndef OBJECT_H
#define OBJECT_H

#include <string>


class UIObject
{
public:
    UIObject(UIObject *parent = nullptr);
    virtual ~UIObject();

    enum ObjectType : unsigned char {
        ApplicationType,
        WindowType,
        DialogType,
        WidgetType,
        PopupType
    };

    UIObject *parent();
    void setParent(UIObject*);
    void setObjectName(const std::wstring&);
    std::wstring objectName();
    virtual void disconnect(int);

protected:
    static int m_connectionId;

private:
    UIObject      *m_parent;
    std::wstring m_object_name;
};

#endif // OBJECT_H
