#include "qrcoutput.h"

#include "qrcparsemethod.h"

namespace RQt{

QrcOutput::QrcOutput()
{

}

/*!
 * @brief 导出qrc资源文件
 * @param[in] rootPath 导出的根文件夹目录
 * @param[in] fullPath 资源文件完整文件名
 * @return true:导出成功，false：导出失败
 */
bool QrcOutput::save(QString rootPath,QString fullPath)
{
    RXmlFile file(fullPath);

    QrcParseMethod method;
    method.setRecources(rootPath,m_resMap);

    file.disableAutoAddXmlSuffix();
    file.setParseMethod(&method,false);
    return file.startSave(QFile::WriteOnly | QFile::Truncate | QFile::Text,false);
}

void QrcOutput::addResources(QString prefix,QStringList files)
{
    m_resMap.insert(prefix,files);
}

} //namespace RQt
