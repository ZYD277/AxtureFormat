#include "mitem.h"

namespace RQt{

MItem::MItem():m_hasAttrRow(false),m_hasAttrColumn(false),m_prop(nullptr)
{

}

MItem::~MItem()
{
    delete m_prop;
    m_prop = nullptr;
}

} //namespace RQt
