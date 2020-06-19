/*!
 *  @brief     回放控制代码模板
 *  @details
 *  @author    wey
 *  @version   1.0
 *  @date      2020.06.18
 *  @warning
 *  @copyright NanJing RenGu.
 */
#ifndef PLAYCONTROLTEMPLATE_H
#define PLAYCONTROLTEMPLATE_H

#include "abstractcodetemplate.h"

namespace CXX{

class PlayControlTemplate : public AbstractCodeTemplate
{
public:
    PlayControlTemplate();

    void prepareOutput(CppGenerate * generate);

    void setStackedId(QString id){m_stackedContainerId = id;}
    void setModelIds(QStringList ids){m_modelIds = ids;}

private:
    QStringList m_modelIds;     /*!< UI控件中模式切换按钮ID */
    QString m_stackedContainerId;       /*!< 外层QStackedWidget的ID */

};

} //namespace CXX

#endif // PLAYCONTROLTEMPLATE_H
