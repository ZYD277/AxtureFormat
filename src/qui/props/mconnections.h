/*!
 *  @brief     信号槽节点属性
 *  @details
 *  @author    wey
 *  @version   1.0
 *  @date      2020.06.05
 *  @warning
 *  @copyright NanJing RenGu.
 */
#ifndef MCONNECTIONS_H
#define MCONNECTIONS_H

#include <QXmlStreamWriter>
#include <QList>

namespace RQt{

class MConnection;

class MConnections
{
public:
    MConnections();
    ~MConnections();

    void write(QXmlStreamWriter & writer,QString tagName);

    void addConn(MConnection * conn);

private:
    QList<MConnection * > m_conns;
};

} //namespace RQt

#endif // MCONNECTIONS_H
