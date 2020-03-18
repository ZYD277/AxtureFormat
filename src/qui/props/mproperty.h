/*!
 *  @brief     ui中property节点
 *  @details
 *  @author    wey
 *  @version   1.0
 *  @date      2020.03.05
 *  @warning
 *  @copyright NanJing RenGu.
 */
#ifndef RPROPERTY_H_2020_03_05
#define RPROPERTY_H_2020_03_05

#include <QString>
#include <QXmlStreamWriter>
#include "mrect.h"

namespace RQt{

class MProperty
{
public:
    MProperty();
    ~MProperty();

    void write(QXmlStreamWriter & writer,QString tagName);

    inline void setAttributeName(QString name){m_attrName = name ; m_bHasAttrName = true;}
    inline QString attributeName()const {return m_attrName;}
    inline bool hasAtributeName()const {return m_bHasAttrName;}

    enum Kind { Unknown = 0, Bool, Color, Cstring, Cursor, CursorShape, Enum, Font, IconSet, Pixmap, Palette, Point,
                Rect, Set, Locale, SizePolicy, Size, String, StringList, Number, Float, Double, Date, Time, DateTime,
                PointF, RectF, SizeF, LongLong, Char, Url, UInt, ULongLong, Brush };
    inline Kind kind() const { return m_kind; }

    void setPropString(QString value);
    QString propString()const{return m_propString;}

    void setPropBool(QString value);
    QString propBool()const {return m_propString;}

    void setPropEnum(QString value);
    QString propEnum()const {return m_propString;}

    void setPropNumber(QString value);
    QString propNumber()const {return m_propString;}

    void setPropSet(QString value);
    QString propSet()const {return m_propString;}

    void setPropRect(MRect * rect);
    MRect * propRect()const {return m_propRect;}

private:
    //attribute
    QString m_attrName;
    bool m_bHasAttrName;

    Kind m_kind;                /*!< 属性类型 */

    QString m_propString;
    MRect * m_propRect;
};

} //namespace RQt

#endif // RPROPERTY_H
