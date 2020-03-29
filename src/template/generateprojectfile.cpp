#include "generateprojectfile.h"

#include <QFile>
#include <QDir>
#include <QDebug>
#include "../util/rutil.h"

GenerateProjectFile::GenerateProjectFile()
{

}

void GenerateProjectFile::setOutputInfo(QString path, QString className,QString qssFileName)
{
    m_projectPath = path;
    m_className = className;
    m_qssFileName = qssFileName;
}

void GenerateProjectFile::startOutput()
{
    outputProFile();
    outputCpp();
    outputMain();
}

void GenerateProjectFile::outputProFile()
{
    QString content = "#-------------------------------------------------\n";
    content += "#\n";
    content += "# Project created by QtCreator"+RUtil::getTimeStamp()+"\n";
    content += "#\n";
    content += "#-------------------------------------------------\n";
    content += "\n";
    content += "QT       += core gui\n";
    content += "\n";
    content += "greaterThan(QT_MAJOR_VERSION, 4): QT += widgets\n";
    content += "\n";
    content += "TARGET = "+m_className+"\n";
    content += "TEMPLATE = app\n";
    content += "\n";
    content += "SOURCES += main.cpp\\ \n";
    content += "           "+m_className+".cpp\n";
    content += "\n";
    content += "HEADERS  += "+m_className+".h\\ \n";
    content += "\n";
    content += "FORMS    += "+m_className+".ui\n";
    content += "\n";
    content += "RESOURCES += res.qrc \n";

    QString fileName = m_projectPath + QDir::separator() + QString("%1.pro").arg(m_className);
    writeToFile(content,fileName);
}

void GenerateProjectFile::outputCpp()
{
    //head
    {
        QString upperClss = m_className.toUpper();
        QString content = "#ifndef "+upperClss+"_H\n";
        content+="#define "+upperClss+"_H\n";
        content+="\n";
        content+="#include <QWidget>\n";
        content+="\n";
        content+="namespace Ui {\n";
        content+="class "+m_className+";\n";
        content+="}\n";
        content+="\n";
        content+="class "+m_className+" : public QWidget\n";
        content+="{\n";
        content+="    Q_OBJECT\n";
        content+="\n";
        content+="public:\n";
        content+="    explicit "+m_className+"(QWidget *parent = 0);\n";
        content+="    ~"+m_className+"();\n";
        content+="\n";
        content+="private:\n";
        content+="    Ui::"+m_className+" *ui;\n";
        content+="};\n";
        content+="\n";
        content+="#endif // "+upperClss+"_H\n";

        QString fileName = m_projectPath + QDir::separator() + QString("%1.h").arg(m_className);
        writeToFile(content,fileName);
    }

    //cpp
    {
        QString content = "#include \""+m_className+".h\"\n";
        content+="#include \"ui_"+m_className+".h\"\n";
        content+="\n";
        content+=(m_className+"::"+m_className+"(QWidget *parent) :\n");
        content+="    QWidget(parent),\n";
        content+="    ui(new Ui::"+m_className+")\n";
        content+="{\n";
        content+="    ui->setupUi(this);\n";
        content+="}\n";
        content+="\n";
        content+=(m_className+"::~"+m_className+"()\n");
        content+="{\n";
        content+="    delete ui;\n";
        content+="}\n";
        content+="\n";

        QString fileName = m_projectPath + QDir::separator() + QString("%1.cpp").arg(m_className);
        writeToFile(content,fileName);
    }
}

void GenerateProjectFile::outputMain()
{
    QString content = "#include \""+m_className+".h\"\n";
    content+="#include <QApplication>\n";
    content+="#include <QFile>\n";
    content+="\n";
    content+="int main(int argc, char *argv[])\n";
    content+="{\n";
    content+="    QApplication a(argc, argv);\n";
    content+="\n";
    content+="    QFile f(\":/style/"+m_qssFileName+"\");\n";
    content+="    if(f.open(QFile::ReadOnly)){\n";
    content+="        a.setStyleSheet(f.readAll());\n";
    content+="    }\n";
    content+="\n";
    content+="    "+m_className+" w;\n";
    content+="    w.showMaximized();\n";
    content+="\n";
    content+="    return a.exec();\n";
    content+="}\n";

    QString fileName = m_projectPath + QDir::separator() + QString("main.cpp");
    writeToFile(content,fileName);
}

void GenerateProjectFile::writeToFile(QString &content, QString fileName)
{
    QFile proFile(fileName);
    if(proFile.open(QFile::WriteOnly)){
        proFile.write(content.toLocal8Bit());
    }
}
