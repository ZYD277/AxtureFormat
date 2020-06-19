/*!
 *  @brief     台位切换代码模板
 *  @details
 *  @author    wey
 *  @version   1.0
 *  @date      2020.06.18
 *  @warning
 *  @copyright NanJing RenGu.
 */
#ifndef TWSWITCHTEMPLATE_H
#define TWSWITCHTEMPLATE_H

#include "abstractcodetemplate.h"

namespace CXX{

class TwSwitchTemplate : public AbstractCodeTemplate
{
public:
    TwSwitchTemplate();

    void prepareOutput(CppGenerate * generate);

    void setIds(QString twPopButtId,QString twContainerId,QString buttContainerId,QList<QPair<QString,QString>> buttIds);

private:
    QList<QPair<QString,QString>> m_buttIds;      /*!< 台位切换按钮组:key:id,value:索引编号 */
    QString m_buttContainerId;  /*!< 台位切换按钮外层容器id */

    QString m_twContainerId;    /*!< 台位切换容器ID */
    QString m_twPopButtId;      /*!< 台位弹出按钮id */

};

} //namespace CXX

#endif // TWSWITCHTEMPLATE_H
