#include "uiobject.h"


int UIObject::m_connectionId = 0;

UIObject::UIObject(UIObject *parent) :
    m_parent(parent)
{

}

UIObject::~UIObject()
{

}

UIObject *UIObject::parent()
{
    return m_parent;
}

void UIObject::setParent(UIObject *parent)
{
    m_parent = parent;
}

void UIObject::setObjectName(const std::wstring &object_name)
{
    m_object_name = object_name;
}

std::wstring UIObject::objectName()
{
    return m_object_name;
}

void UIObject::disconnect(int connectionId)
{

}
