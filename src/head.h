#ifndef HEAD_H
#define HEAD_H

#include <QString>
#include <QList>

#define Check_Return(condition,returnVal) {if(condition) return returnVal;}
#define Check_Return_Cb(condition,returnVal,callback){if(condition){ callback(); return returnVal;}}

#define Table_Icon_Width 80     //表格图标列宽

/**
 * @brief 进度条信息
 */
struct ProcessBarData{
    ProcessBarData():m_progress(0),error(false){}
    int m_progress;             /*!< 转换进度0-100 */
    QString textDescription;    /*!< 转换过程中文描述 */
    bool error;                 /*!< 转换过程中是否产生错误，若产生错误则将进度条标记为红色 */
    int finishedTime;
    int currentTime;
};

/*!
 * @brief Axuture工程中单个页面文件信息
 */
struct AxturePage{
    QString htmlFilePath;
    QString jsFilePath;
    QString cssFilePath;
    QString htmlFileName;
    ProcessBarData processData;
};

typedef QList<AxturePage> AxturePages;

/*!
 * @brief 单个axture工程文件夹包含的文件信息
 */
struct AxtureProject{
    QString projectName;            /*!< axure项目名 */
    QString projectPath;            /*!< auxre项目绝对路径 */

    QString baseJsFilePath;
    QString baseCssFilePath;        /*!< axture通用样式文件 */

    AxturePages pages;
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
