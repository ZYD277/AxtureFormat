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

class GenerateProjectFile
{
public:
    GenerateProjectFile();

    void setOutputInfo(QString path, QString className, QString qssFileName);

    void startOutput();

private:
    void outputProFile();
    void outputCpp();
    void outputMain();

    void writeToFile(QString & content,QString fileName);

private:
    QString m_projectPath;
    QString m_className;
    QString m_qssFileName;
};

#endif // GEMERATEPROJECTFILE_H
