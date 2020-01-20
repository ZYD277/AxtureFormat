#ifndef QRCOUTPUT_H
#define QRCOUTPUT_H

#include <QString>
#include <QStringList>
#include <QMap>

namespace RQt{

class QrcParseMethod;

class QrcOutput
{
public:
    QrcOutput();

    void addResources(QString prefix,QStringList files);

    bool save(QString fullPath);

private:
    QrcParseMethod * m_parseMethod;

    QMap<QString,QStringList>  m_resMap;  /*!< key:prefix，value:对应的资源列表 */
};

} //namespace RQt

#endif // QRCOUTPUT_H
