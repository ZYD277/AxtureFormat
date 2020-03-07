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
#include <QXmlStreamWriter>

namespace RQt{

class MProperty;

class MItem
{
public:
    MItem();
    ~MItem();

    void write(QXmlStreamWriter & writer,QString tagName);

    void setAttributeRow(QString row){m_attrRow = row;m_hasAttrRow = true;}
    bool hasAttributeRow()const{return m_hasAttrRow;}
    QString attributeRow()const{return m_attrRow;}

    void setAttributeColumn(QString column){m_attrColumn = column;m_hasAttrColumn = true;}
    bool hasAttributeColumn()const{return m_hasAttrColumn;}
    QString attributeColumn()const{return m_attrColumn;}

    void setProperty(MProperty * prop){m_prop = prop;}
    void addItem(MItem * item){m_items.append(item);}

private:
    bool m_hasAttrRow;
    QString m_attrRow;

    bool m_hasAttrColumn;
    QString m_attrColumn;

    MProperty * m_prop;
    QList<MItem *> m_items;
};

} //namespace RQt

#endif // RITEM_H
