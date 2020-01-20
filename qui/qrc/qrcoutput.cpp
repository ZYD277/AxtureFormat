#include "qrcoutput.h"

#include "qrcparsemethod.h"

namespace RQt{

QrcOutput::QrcOutput()
{

}

bool QrcOutput::save(QString fullPath)
{
    RXmlFile file(fullPath);

    QrcParseMethod method;
    method.setRecources(m_resMap);

    file.disableAutoAddXmlSuffix();
    file.setParseMethod(&method,false);
    return file.startSave(QFile::WriteOnly | QFile::Truncate | QFile::Text,false);
}

void QrcOutput::addResources(QString prefix,QStringList files)
{
    m_resMap.insert(prefix,files);
}

} //namespace RQt
