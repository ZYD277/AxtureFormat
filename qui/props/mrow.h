/*!
 *  @brief     表格行属性
 *  @details
 *  @author    wey
 *  @version   1.0
 *  @date      2020.03.05
 *  @warning
 *  @copyright NanJing RenGu.
 */
#ifndef MROW_H
#define MROW_H

#include <QString>
#include <QList>

namespace RQt{

class MProperty;

class MRow
{
public:
    MRow();
    ~MRow();

    void addProperty(MProperty * prop);

private:
    QList<MProperty *> m_props;

};

} //namespace RQt

#endif // MROW_H
