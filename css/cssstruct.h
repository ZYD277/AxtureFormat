#ifndef CSSSTRUCT_H
#define CSSSTRUCT_H

#include <QString>
#include <QMap>
#include <QList>

namespace CSS{

/*!
 * @brief 选择器类型
 * @details id选择器、class选择器、元素选择器等
 */
enum SelectorType{
    Clazz,      //.xx
    Id,         //#xx
    Element     //p、body
};

/*!
 * @brief 单条css规则
 */
struct CssRule{
    QString name;
    QString value;
};

typedef QList<CssRule> Rules;

/*!
 * @brief 单个选择器及规则集合
 */
struct CssSegment{
    SelectorType type;        /*!< 选择器类型 */
    QString selectorName;     /*!< 选择器名称 */
    Rules rules;              /*!< 规则集合 */

    bool isId(){return type == Id;}
    bool isClazz(){return type == Clazz;}
    bool isElement(){return type == Element;}
};

typedef QMap<QString,CssSegment> CssMap;

}  //namespace CSS

#endif // CSSSTRUCT_H
