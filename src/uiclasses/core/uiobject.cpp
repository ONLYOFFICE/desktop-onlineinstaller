#include "uiobject.h"


int UIObject::m_connectionId = 0;

UIObject::UIObject(ObjectType type, UIObject *parent) :
    m_parent(parent),
    m_objectType(type),
    m_groupId({})
{

}

UIObject::~UIObject()
{

}

void UIObject::setParent(UIObject *parent) noexcept
{
    m_parent = parent;
}

void UIObject::setObjectName(const tstring &object_name)
{
    m_objectName = object_name;
}

void UIObject::setObjectGroupId(const tstring &id)
{
    m_groupId = id;
}

UIObject *UIObject::parent() const noexcept
{
    return m_parent;
}

UIObject::ObjectType UIObject::objectType() const noexcept
{
    return m_objectType;
}

tstring UIObject::objectName() const noexcept
{
    return m_objectName;
}

tstring UIObject::objectGroupId() const noexcept
{
    return m_groupId;
}

void UIObject::disconnect(int connectionId)
{

}
