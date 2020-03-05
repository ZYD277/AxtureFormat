#include "mproperty.h"

namespace RQt{

MProperty::MProperty():m_kind(Unknown),m_propRect(nullptr)
{

}

MProperty::~MProperty()
{
    delete m_propRect;
    m_propRect = nullptr;
}

void MProperty::setPropString(QString value)
{
    m_kind = Cstring;
    m_propString = value;
}

void MProperty::setPropBool(QString value)
{
    m_kind = Bool;
    m_propString = value;
}

void MProperty::setPropEnum(QString value)
{
    m_kind = Enum;
    m_propString = value;
}

void MProperty::setPropNumber(QString value)
{
    m_kind = Number;
    m_propString = value;
}

void MProperty::setPropRect(MRect *rect)
{
    m_kind = Rect;
    m_propRect = rect;
}

} //namespace RQt
