/*!
 *  @brief     资源文件节点属性
 *  @details
 *  @author    wey
 *  @version   1.0
 *  @date      2020.03.06
 *  @warning
 *  @copyright NanJing RenGu.
 */
#ifndef MDOMRESOURCE_H
#define MDOMRESOURCE_H

#include <QXmlStreamWriter>
#include <QList>

namespace RQt{

class MDomResource
{
public:
    MDomResource();
    ~MDomResource();

    void write(QXmlStreamWriter & writer,QString tagName);

    void addResource(QString resourcePath);

private:
    QList<QString> m_ress;
};

} //namespace RQt

#endif // RDOMRESOURCE_H
