#ifndef MATTRIBUTE_H
#define MATTRIBUTE_H

#include <QString>
#include <QXmlStreamWriter>
#include "mrect.h"

namespace RQt{

class MAttribute
{
public:
    MAttribute();
    ~MAttribute();

    void write(QXmlStreamWriter & writer,QString tagName);

    inline void setAttributeName(QString name){m_attrName = name ; m_bHasAttrName = true;}
    inline QString attributeName()const {return m_attrName;}
    inline bool hasAtributeName()const {return m_bHasAttrName;}

    enum Kind { Unknown = 0, Bool, Color, Cstring, Cursor, CursorShape, Enum, Font, IconSet, Pixmap, Palette, Point,
                Rect, Set, Locale, SizePolicy, Size, String, StringList, Number, Float, Double, Date, Time, DateTime,
                PointF, RectF, SizeF, LongLong, Char, Url, UInt, ULongLong, Brush };
    inline Kind kind() const { return m_kind; }

    void setAttributeBool(QString value);
    QString propBool()const {return m_propString;}

    void setPropNumber(QString value);
    QString propNumber()const {return m_propString;}

    void setPropString(QString value);
    QString propString()const {return m_propString;}

private:
    QString m_attrName;
    bool m_bHasAttrName;

    Kind m_kind;                /*!< 属性类型 */

    QString m_propString;
};

} //namespace RQt
#endif // MATTRIBUTE_H
