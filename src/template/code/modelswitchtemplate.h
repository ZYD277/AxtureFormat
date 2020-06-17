/*!
 *  @brief     模式切换按钮组合代码模板
 *  @details
 *  @author    wey
 *  @version   1.0
 *  @date      2020.06.17
 *  @warning
 *  @copyright NanJing RenGu.
 */
#ifndef MODELSWITCHTEMPLATE_H
#define MODELSWITCHTEMPLATE_H

#include "abstractcodetemplate.h"

namespace CXX{

class ModelSwitchTemplate : public AbstractCodeTemplate
{
public:
    ModelSwitchTemplate();

    void prepareOutput(CppGenerate * generate);

    void setModelIds(QStringList ids){m_modelIds = ids;}

private:
    QStringList m_modelIds;     /*!< UI控件中模式切换按钮ID */

};

} //namespace CXX

#endif // MODELSWITCHTEMPLATE_H
