/*!
 *  @brief     ui中Widget节点
 *  @details
 *  @author    wey
 *  @version   1.0
 *  @date      2020.03.05
 *  @warning
 *  @copyright NanJing RenGu.
 */
#ifndef RDOMWIDGET_H
#define RDOMWIDGET_H

#include <QString>
#include <QList>

namespace RQt{

class MProperty;
class MRow;
class MColumn;
class MItem;

class RDomWidget
{
public:
    RDomWidget();
    ~RDomWidget();

    /*!< 属性操作 */
    inline void setAttributeName(QString id){m_attrName = id;m_bHasAttrName = true;}
    inline bool hasAttributeName()const {return m_bHasAttrName;}
    inline QString attributeName()const {return m_attrName;}

    inline void setAttributeClass(QString clazz){m_attrClass = clazz;m_bHasAttrClass = true;}
    inline bool hasAttributeClass()const {return m_bHasAttrClass;}
    inline QString attributeClass()const {return m_attrClass;}

    void addProperty(MProperty * prop);
    QList<MProperty *> props(){return m_props;}

    void addWidget(RDomWidget * widget);
    QList<RDomWidget *> widgets(){return m_widgets;}

    void addRow(MRow * row);
    QList<MRow *> rows(){return m_rows;}

    void addColumn(MColumn * column);
    QList<MColumn *> columns(){return m_columns;}

    void addItem(MItem * item);
    QList<MItem *> items(){return m_items;}

private:
    //attribute
    QString m_attrName;
    bool m_bHasAttrName;

    QString m_attrClass;
    bool m_bHasAttrClass;

    QList<MProperty *> m_props;
    QList<RDomWidget *> m_widgets;
    QList<MRow *> m_rows;
    QList<MColumn *> m_columns;
    QList<MItem *> m_items;

};

} //namespace RQt

#endif // RDOMWIDGET_H
