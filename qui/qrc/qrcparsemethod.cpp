#include "qrcparsemethod.h"

#include <QDebug>

namespace RQt{

QrcParseMethod::QrcParseMethod()
{

}

void QrcParseMethod::setRecources(QMap<QString, QStringList> resMap)
{
    m_resMap = resMap;
}

bool QrcParseMethod::startSave(QDomDocument &doc)
{
    QDomElement rcc = doc.createElement("RCC");

    auto iter = m_resMap.begin();
    while(iter != m_resMap.end()){

        QDomElement qres = doc.createElement("qresource");
        qres.setAttribute("prefix",iter.key());

        foreach(const QString & fpath,iter.value()){
            QDomElement fele = doc.createElement("file");
            fele.appendChild(doc.createTextNode(fpath));

            qres.appendChild(fele);
        }

        rcc.appendChild(qres);

        iter++;
    }

    doc.appendChild(rcc);
    return true;
}

} //namespace RQt
