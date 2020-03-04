#ifndef GLOBAL_H
#define GLOBAL_H

#include <QObject>
#include <QList>

namespace Global
{
enum RowName{
    COLUMN_NUMBER = 0,
    COLUMN_FILENAME = 1,
    COLUMN_STATE = 2,
    COLUMN_OPEN = 3,
    COLUMN_DELETE = 4,
    COLUMN_SWITCH = 5,
    COLUMN_STATEFINISH = 6
};
/**
 * @brief 文件夹信息结构体
 */
struct DirInfo
{
    QString m_dirPath;
    QString m_dirName;
};
/**
 * @brief 文件信息结构体
 */
struct FileInfo
{
    QString m_fielPath;
    QString m_fileName;
    QString m_filesuffix;
    QString m_fileBaseName;
    int m_fishProcessTime;
    bool m_succeedSign;
    int m_currentProcessTime;
};
/**
 * @brief 转换所需版本信息
 */
struct Stru_SwitchNeedMsg
{
    QString m_axureVersion;
    QString m_QtVersion;
    QString m_rootDirectory;
    QString m_newDirectory;
};
//extern QList<FileInfo> m_suitableFileList;
}

#endif // GLOBAL_H
