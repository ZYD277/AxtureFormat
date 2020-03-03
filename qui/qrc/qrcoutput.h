/*!
 *  @brief     qrc资源格式输出
 *  @details   可多线程调用
 *  @author    wey
 *  @version   1.0
 *  @date      2020.03.02
 *  @warning
 *  @copyright NanJing RenGu.
 */
#ifndef QRCOUTPUT_H
#define QRCOUTPUT_H

#include <QString>
#include <QStringList>
#include <QMap>

namespace RQt{

class QrcOutput
{
public:
    QrcOutput();

    void addResources(QString prefix,QStringList files);

    bool save(QString fullPath);

private:
    QMap<QString,QStringList>  m_resMap;  /*!< key:prefix，value:对应的资源列表 */
};

} //namespace RQt

#endif // QRCOUTPUT_H
