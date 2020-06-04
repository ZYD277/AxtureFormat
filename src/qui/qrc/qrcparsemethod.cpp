#include "qrcparsemethod.h"

#include <QDebug>
#include <QFileInfo>
#include <QDir>

namespace RQt{

QrcParseMethod::QrcParseMethod()
{

}

void QrcParseMethod::setRecources(QString rootPath,QMap<QString, QStringList> resMap)
{
    m_exportRootPath = rootPath;
    m_resMap = resMap;
}

/*!
 * @attention 保存时对文件是否存在进行检测，只有存在后才会写入资源文件
 */
bool QrcParseMethod::startSave(QDomDocument &doc)
{
    QDomElement rcc = doc.createElement("RCC");

    auto iter = m_resMap.begin();

    while(iter != m_resMap.end()){
        QDomElement qres = doc.createElement("qresource");
        qres.setAttribute("prefix",iter.key());

        foreach(const QString & fpath,iter.value()){
            QString fullpath = m_exportRootPath + QDir::separator() + fpath;

            if(QFileInfo(fullpath).exists()){
                QDomElement fele = doc.createElement("file");
                fele.appendChild(doc.createTextNode(fpath));

                qres.appendChild(fele);
            }
        }

        rcc.appendChild(qres);

        iter++;
    }

    doc.appendChild(rcc);
    return true;
}

} //namespace RQt
