#include "mconnection.h"

namespace RQt{

MConnection::MConnection()
{

}

MConnection::~MConnection()
{

}

void MConnection::write(QXmlStreamWriter &writer, QString tagName)
{
    writer.writeStartElement(tagName.isEmpty() ? "connection" : tagName);

    writer.writeTextElement("sender",m_sender);
    writer.writeTextElement("signal",m_signal);
    writer.writeTextElement("receiver",m_receiver);
    writer.writeTextElement("slot",m_slot);

    writer.writeEndElement();
}

void MConnection::setSender(QString sender)
{
    m_sender = sender;
}

void MConnection::setSignal(QString signal)
{
    m_signal = signal;
}

void MConnection::setReceiver(QString receiver)
{
    m_receiver = receiver;
}

void MConnection::setSlot(QString slot)
{
    m_slot = slot;
}

} //namespace RQt
