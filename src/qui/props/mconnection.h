/*!
 *  @brief     信号槽节点属性
 *  @details
 *  @author    wey
 *  @version   1.0
 *  @date      2020.06.05
 *  @warning
 *  @copyright NanJing RenGu.
 */
#ifndef MCONNECTION_H
#define MCONNECTION_H

#include <QXmlStreamWriter>
#include <QList>

namespace RQt{

class MConnection
{
public:
    MConnection();
    ~MConnection();

    void write(QXmlStreamWriter & writer,QString tagName);

    void setSender(QString sender);
    void setSignal(QString signal);

    void setReceiver(QString receiver);
    void setSlot(QString slot);

private:
    QList<QString> m_ress;

    QString m_sender;
    QString m_signal;
    QString m_receiver;
    QString m_slot;

};

} //namespace RQt

#endif // MCONNECTION_H
