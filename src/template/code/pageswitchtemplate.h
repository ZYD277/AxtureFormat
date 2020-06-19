/*!
 *  @brief     翻页控件代码生成
 *  @details
 *  @author    wey
 *  @version   1.0
 *  @date      2020.06.18
 *  @warning
 *  @copyright NanJing RenGu.
 */
#ifndef PAGESWITCHTEMPLATE_H
#define PAGESWITCHTEMPLATE_H

#include "abstractcodetemplate.h"

namespace CXX{

class PageSwitchTemplate : public AbstractCodeTemplate
{
public:
    PageSwitchTemplate();

    void prepareOutput(CppGenerate * generate);

    void setIds(QString leftId,QString rightId,QStringList pageIds);

private:
    QStringList m_pageIds;
    QString m_leftPageId;
    QString m_rightPageId;

};

} //namespace CXX

#endif // PAGESWITCHTEMPLATE_H
