#ifndef QRCPARSEMETHOD_H
#define QRCPARSEMETHOD_H

#include <QStringList>
#include <QMap>

#include "../../util/fileutils.h"

namespace RQt{

class QrcParseMethod : public RXmlParseMethod
{
public:
    QrcParseMethod();

    void setRecources(QMap<QString,QStringList> resMap);

    bool  startSave(QDomDocument & doc) override;

private:
    QMap<QString,QStringList>  m_resMap;  /*!< key:prefix，value:对应的资源列表 */
};

} //namespace RQt

#endif // QRCPARSEMETHOD_H
