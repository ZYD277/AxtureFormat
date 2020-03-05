/*!
 *  @brief     控件位置尺寸属性
 *  @details
 *  @author    wey
 *  @version   1.0
 *  @date      2020.03.05
 *  @warning
 *  @copyright NanJing RenGu.
 */
#ifndef RRECT_H
#define RRECT_H

#include <QRect>

namespace RQt{

class MRect{
public:
    MRect();
    ~MRect();

    void setRect(QRect rect);
    const QRect &rect()const;

private:
    QRect m_rect;
};

} //namespace RQt

#endif // RRECT_H
