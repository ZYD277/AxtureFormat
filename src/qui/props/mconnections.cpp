#include "mconnections.h"

#include "mconnection.h"

namespace RQt{

MConnections::MConnections()
{

}

MConnections::~MConnections()
{
    for(int i = 0; i < m_conns.size(); i++){
        delete m_conns.at(i);
    }
}

void MConnections::write(QXmlStreamWriter &writer, QString tagName)
{
    writer.writeStartElement(tagName.isEmpty() ? "connections" : tagName);

    foreach(MConnection * conn,m_conns){
        conn->write(writer,"connection");
    }

    writer.writeEndElement();
}

void MConnections::addConn(MConnection *conn)
{
    if(conn)
        m_conns.append(conn);
}



} //namespace RQt
