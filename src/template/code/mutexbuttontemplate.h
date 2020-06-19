/*!
 *  @brief     互斥按钮代码模板
 *  @details
 *  @author    wey
 *  @version   1.0
 *  @date      2020.06.19
 *  @warning
 *  @copyright NanJing RenGu.
 */
#ifndef MUTEXBUTTONTEMPLATE_H
#define MUTEXBUTTONTEMPLATE_H

#include "abstractcodetemplate.h"

namespace CXX{

class MutexButtonTemplate : public AbstractCodeTemplate
{
public:
    MutexButtonTemplate();

    void prepareOutput(CppGenerate * generate);

    void setMutexButtonIds(QStringList mutexButts);

private:
    QStringList m_mutexButtIds;     /*!< 互斥按钮id集合 */

};

} //namespace CXX

#endif // MUTEXBUTTONTEMPLATE_H
