#ifndef HEAD_H
#define HEAD_H

#include <QString>
#include <QList>
#include "util/rutil.h"

#define Check_Return(condition,returnVal) {if(condition) return returnVal;}
#define Check_Return_Cb(condition,returnVal,callback){if(condition){ callback(); return returnVal;}}

#define Table_Icon_Width 80     //表格图标列宽

/**
 * @brief 更新进度
 */
struct ProcessBarData{
    ProcessBarData():m_progress(0),error(false){}
    int m_progress;             /*!< 转换进度0-100 */
    bool error;                 /*!< 转换过程中是否产生错误，若产生错误则将进度条标记为红色 */
    QString textDescription;    /*!< 转换过程中文描述 */
};

/*!
 * @brief Axuture工程中单个页面文件信息
 */
struct AxurePage{
    AxurePage(){
        id = RUtil::UUID();
    }

    QString id;     /*!< 页面uuid */

    QString baseJsFilePath;
    QString baseCssFilePath;        /*!< axture通用样式文件 */

    QString htmlFilePath;           /*!< 页面html绝对路径 */
    QString jsFilePath;             /*!< 当前html页面依赖的js文件路径 */
    QString cssFilePath;            /*!< 当前html页面依赖的css文件路径 */
    QString htmlFileName;
    ProcessBarData processData;
};

typedef QList<AxurePage> AxturePages;

/*!
 * @brief 单个页面转换进度
 */
struct SwitchProgress{
    QString pageId;             /*!< 页面Id */
    QString pageName;
    ProcessBarData progress;
};

/*!
 * @brief 表格列
 */
enum TColumn{
    T_Index,
    T_FileName,
    T_Progress,
    T_Open,
    T_Delete,
    T_Switch,
    T_ColumnCount,   /*!< 表格总行 */

    T_Description,   /*!< 转换过程 */
    T_OccurrError    /*!< 转换过程中是否发生错误 */
};



#endif // HEAD_H
