/*!
 *  @brief     表格列属性
 *  @details
 *  @author    wey
 *  @version   1.0
 *  @date      2020.03.05
 *  @warning
 *  @copyright NanJing RenGu.
 */
#ifndef MCOLUMN_H
#define MCOLUMN_H

#include <QList>

namespace RQt{

class MProperty;

class MColumn
{
public:
    MColumn();
    ~MColumn();

    void addProperty(MProperty * prop);

private:
    QList<MProperty *> m_props;

};

} //namespace RQt

#endif // MCOLUMN_H
