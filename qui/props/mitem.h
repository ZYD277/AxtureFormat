/*!
 *  @brief     表格或树形控件单元格树形
 *  @details
 *  @author    wey
 *  @version   1.0
 *  @date      2020.03.05
 *  @warning
 *  @copyright NanJing RenGu.
 */
#ifndef RITEM_H
#define RITEM_H

#include <QString>

namespace RQt{

class MProperty;

class MItem
{
public:
    MItem();
    ~MItem();

    void setAttributeRow(QString row){m_attrRow = row;m_hasAttrRow = true;}
    QString attributeRow()const{return m_attrRow;}

    void setAttributeColumn(QString column){m_attrColumn = column;m_hasAttrColumn = true;}
    QString attributeColumn()const{return m_hasAttrColumn;}

    void setProperty(MProperty * prop){m_prop = prop;}

private:
    bool m_hasAttrRow;
    QString m_attrRow;

    bool m_hasAttrColumn;
    QString m_attrColumn;

    MProperty * m_prop;

};

} //namespace RQt

#endif // RITEM_H
