/*!
 *  @brief     qrc格式导出至文件
 *  @details
 *  @author    wey
 *  @version   1.0
 *  @date      2020.03.02
 *  @warning
 *  @copyright NanJing RenGu.
 */
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

    void setRecources(QString rootPath, QMap<QString,QStringList> resMap);

    bool  startSave(QDomDocument & doc) override;

private:
    QString m_exportRootPath;             /*!< 导出的文件夹根目录 */
    QMap<QString,QStringList>  m_resMap;  /*!< key:prefix，value:对应的资源列表 */
};

} //namespace RQt

#endif // QRCPARSEMETHOD_H
