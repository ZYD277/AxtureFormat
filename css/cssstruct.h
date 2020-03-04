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
    Clazz,          //.xx
    Id,             //#xx
    DynamicType,    //#xx.mouseOver
    Element         //p、body
};

/*!
 * @brief 动态属性类型
 */
enum DynamicPropType{
    DInvalid,
    DMouseOver,
    DMouseDown,
    DSelected,
    DDisabled
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
 * @details 1.css中包含多种选择器类型，如#u382_div {}，#表示选择器类型，u382_div表示选择器名称，{}内的以';'分割的为单条规则
 *          2.axure中的控件可添加interaction styles(交互式样式)，包括MouseOver、MouseDown、Selected、DIsabled等动态属性 \n
 *            这些动态样式需要配合鼠标操作进行显示，在qt中也可以实现类似的效果，但需要进行特化处理。在axure的pushbutton上加入mousedown状态，\n
 *            在css中生成的代码为#u382_div.mouseDown{}，可以将上面转换成QPushButton#u382_div[dynamicProp="mouseDown"]{}
 */
struct CssSegment{
    SelectorType type;        /*!< 选择器类型 */
    QString selectorName;     /*!< 选择器名称 */
    Rules rules;              /*!< 规则集合 */
    DynamicPropType dtype;    /*!< 动态属性类型,只在type为DynamicType时有效 */

    bool isId(){return type == Id;}
    bool isClazz(){return type == Clazz;}
    bool isElement(){return type == Element;}
};

/*!
 *@brief 解析异常信息提示
 */
struct ErrorMsg{
    bool parse;                 /*!< 解析是否成功*/
    QString errorMsg;        /*!< 解析故障信息提示*/
};

typedef QMap<QString,CssSegment> CssMap;        /*!< key:选择器名称，value:对应属性规则 */

}  //namespace CSS

#endif // CSSSTRUCT_H
