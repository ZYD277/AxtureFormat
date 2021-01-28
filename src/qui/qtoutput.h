#ifndef QTOUTPUT_H
#define QTOUTPUT_H

#include <QSize>

#include "../html/htmlstruct.h"
#include "../css/cssstruct.h"
#include "../qui/formatproperty.h"
#include "qss/qssoutput.h"
#include "../template/cppstruct.h"

namespace RQt{

class QtOutput
{
public:
    QtOutput();
    ~QtOutput();

    bool save(QString className, DomHtmlPtr ptr, CSS::CssMap globalCss, CSS::CssMap pageCss, QString fullPath);
    bool saveQss(QString cssFileName);
    bool saveQrc();

    QStringList getOriginalResouces(){return m_originalResoucesLinks;}
    CXX::CppCodeDatas getCppCodeDatas(){return m_codeDatas;}

    QStringList getCustomClassList(){return m_customClassList;}

    QSize getWindowMinimumSize(){return m_minimumSize;}


private:
    QStringList m_originalResoucesLinks;            /*!< 原始图片资源连接 */
    SelectorTypeMap m_selectorType;
    CXX::CppCodeDatas m_codeDatas;      /*!< 需生成代码的控件信息集合 */

    FormatProperty propFormat;
    QSSOutput qssOutput;
    QString m_qssFileName;              /*!< qss保存文件名 */
    QString m_saveFullFilePath;         /*!< ui文件的完整路径 */

    QStringList m_customClassList;       /*!< 保存控件提升的自定义类 */

    QSize m_minimumSize;         /*!< 设置窗口尺寸 */

};

} //namespace RQt

#endif // QTOUTPUT_H
