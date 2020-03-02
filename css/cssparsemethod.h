/*!
 *  @brief     CSS
 *  @details   1.解析axure生成的css格式文件；
 *             2.支持动态样式解析提取
 *  @author    tzj
 *  @version   1.0
 *  @date      2020.02.26
 *  @warning
 *  @copyright NanJing RenGu.
 *  @note 20200302:wey:修复无法去除多个注释信息;
 */
#ifndef CSSPARSEMETHOD_H
#define CSSPARSEMETHOD_H

#include <QSharedPointer>
#include "../util/fileutils.h"
#include "cssstruct.h"

namespace CSS{

class CssParseMethod : public RTextParseMethod
{
public:
    CssParseMethod();

    bool  startParse(RTextFile * file) override;

    CssMap getParsedResult(){return m_cssMap;}

    ErrorMsg getParsedErrorMsg(){return m_errorMsg;}

private:
    QString specialDis(const QString &character, int startPosition, int endPosition);    /*!< 处理数据中的符号*/
    bool dataKey(const QString &keyData);      /*!< key的特殊处理*/
    bool removeComments(QString &originData);

    bool parseFile(RTextFile * file);

    DynamicPropType getDynamicType(QString dyname);

private:
    /*!
     * @brief 一个多行注释位置记录信息
     */
    struct MultiComment{
        int startPos;       /*!< 注释开始位置 */
        int startCount;     /*!< 注释中*个数，若<=1，则注释错误 */
        int endPos;         /*!< 注释结束位置 */
    };

private:
    CssMap m_cssMap;

    ErrorMsg m_errorMsg;

    QMap<QString,DynamicPropType> m_interateStyleMap;           /*!< key:属性名，value:动态类型 */

};

} //namespace Css

#endif // CSSPARSEMETHOD_H
