#ifndef HEAD_H
#define HEAD_H

#include <QString>
#include <QList>

#define Check_Return(condition,returnVal) {if(condition) return returnVal;}
#define Check_Return_Cb(condition,returnVal,callback){if(condition){ callback(); return returnVal;}}

/*!
 * @brief Axuture工程中单个页面文件信息
 */
struct AxturePage{
    QString htmlFilePath;
    QString jsFilePath;
    QString cssFilePath;
};

typedef QList<AxturePage> AxturePages;

/*!
 * @brief 单个axture工程文件夹包含的文件信息
 */
struct AxtureProject{
    QString projectName;

    QString baseJsFilePath;
    QString baseCssFilePath;        /*!< axture通用样式文件 */

    AxturePages pages;
};



#endif // HEAD_H
