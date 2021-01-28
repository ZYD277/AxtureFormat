/*!
 *  @brief     在指定目录下生成项目工程源码
 *  @details   为了便于用户查看转换后效果，工具自动为用户生成模板代码
 *  @author    wey
 *  @version   1.0
 *  @date      2020.03.27
 *  @warning
 *  @copyright NanJing RenGu.
 */
#ifndef GEMERATEPROJECTFILE_H
#define GEMERATEPROJECTFILE_H

#include <QString>
#include <QSize>

#include "cppstruct.h"

class GenerateProjectFile
{
public:
    GenerateProjectFile();

    void setCodeDatas(CXX::CppCodeDatas datas);
    void setOutputInfo(QString path, QString className, QString qssFileName);

    void setCustomControlPath(QString path, QStringList customClassList);

    void startOutput(bool generateCode);

    void setWindMinimumSize(QSize miniSize);

private:
    void outputProFile();
    void outputCpp(bool generateCode);
    void outputMain();

    QString addCustomControlCppFile();
    QString addCustomControlHeadFile();

    void writeToFile(QString content, QString fileName);

    void writeToBomFile(QString content, QString fileName);

private:
    QString m_projectPath;
    QString m_className;
    QString m_qssFileName;

//    QString m_customControlPath;
    QStringList m_customHeadNames;
    QStringList m_customCppNames;

    CXX::CppCodeDatas m_codeDatas;

    QSize m_minimumSize;
};

#endif // GEMERATEPROJECTFILE_H
