/*!
 *  @brief     Cpp文件导出
 *  @details   此模板允许各个类型的代码块插入，在最终导出的时候进行回调
 *  @author    wey
 *  @version   1.0
 *  @date      2020.06.17
 *  @warning
 *  @copyright NanJing RenGu.
 */
#ifndef CPPGENERATE_H
#define CPPGENERATE_H

#include <QString>
#include <QMap>
#include <QSet>

#include "cppstruct.h"

namespace CXX{

class CppGenerate
{
public:
    CppGenerate();
    ~CppGenerate(){}

    QString getHeadContent();
    QString getCppContent();

    void addPrivateSlot(QString slot){m_privateSlots.append(slot);}
    void addPrivateFun(QString func){m_privateFuns.append(func);}
    void addPrivateMember(QString member){m_privateMembers.append(member);}

    void addInclude(QString includeHead){m_includes.insert(includeHead);}

    void addConstructInitCode(QString code){m_constructInitCode.append(code);}
    void addFunImplement(QString code){m_funImplements.append(code);}

    void setClssName(QString clazzName){m_clazzName = clazzName;}
    QString getClassName(){return m_clazzName;}

private:
    QStringList m_privateSlots;         /*!< 私有槽函数 */
    QStringList m_privateFuns;          /*!< 私有函数列表 */
    QStringList m_privateMembers;       /*!< 私有成员变量列表 */

    QSet<QString> m_includes;             /*!< 所需要包含的头文件列表 */

    QStringList m_constructInitCode;    /*!< 构造函数初始化时执行的代码 */
    QStringList m_funImplements;        /*!< 函数实现代码 */

    QString m_clazzName;                /*!< cpp中类名称 */
};

};

#endif // CPPGENERATE_H
