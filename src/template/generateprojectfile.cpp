﻿#include "generateprojectfile.h"

#include <QFile>
#include <QDir>
#include <QDebug>
#include "../util/rutil.h"
#include "cppgenerate.h"
#include "code/modelswitchtemplate.h"

QString NEW_EMPTY = "";
QString New_Line = "\n";
QString New_Space = "\t";

QString ConcatNewLine(QString text){
    return text += New_Line;
}

QString ConcatTabSpace(QString text){
    return New_Space + text;
}

GenerateProjectFile::GenerateProjectFile()
{

}

void GenerateProjectFile::setCodeDatas(CXX::CppCodeDatas datas)
{
    m_codeDatas = datas;
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
    QString content = ConcatNewLine("#-------------------------------------------------");
    content += ConcatNewLine("#");
    content += ConcatNewLine("# Project created by QtCreator"+RUtil::getTimeStamp());
    content += ConcatNewLine("#");
    content += ConcatNewLine("#-------------------------------------------------");
    content += New_Line;
    content += ConcatNewLine("QT       += core gui");
    content += New_Line;
    content += ConcatNewLine("greaterThan(QT_MAJOR_VERSION, 4): QT += widgets");
    content += New_Line;
    content += ConcatNewLine("TARGET = " + m_className);
    content += ConcatNewLine("TEMPLATE = app");
    content += New_Line;
    content += ConcatNewLine("SOURCES += main.cpp\\ ");
    content += ConcatNewLine("           " + m_className+".cpp");
    content += New_Line;
    content += ConcatNewLine("HEADERS  += " + m_className+".h\\ ");
    content += New_Line;
    content += ConcatNewLine("FORMS    += " + m_className+".ui");
    content += New_Line;
    content += ConcatNewLine("RESOURCES += res.qrc ");

    QString fileName = m_projectPath + QDir::separator() + QString("%1.pro").arg(m_className);
    writeToFile(content,fileName);
}

void GenerateProjectFile::outputCpp()
{
    CXX::CppGenerate cpp;
    cpp.setClssName(m_className);

    QMap<CXX::CodeType,int> t_typeStatics;

    //生成插件对应的代码
    foreach (CXX::AbstractCppCodeData * codeData, m_codeDatas) {

        if(t_typeStatics.contains(codeData->m_type)){
            t_typeStatics.operator [](codeData->m_type)++;
        }else{
            t_typeStatics.insert(codeData->m_type,1);
        }

        int typeIndex = t_typeStatics.value(codeData->m_type);

        switch(codeData->m_type){
            case CXX::MODEL_SWITCH:{
                CXX::ModelSwitchCodeData * switchData = dynamic_cast<CXX::ModelSwitchCodeData *>(codeData);

                CXX::ModelSwitchTemplate modelSwitch;
                modelSwitch.setSameTypeIndex(typeIndex);
                modelSwitch.setModelIds(switchData->m_modelIds);
                modelSwitch.prepareOutput(&cpp);
            }
                break;

            default:break;
        }

        delete codeData;
    }

    //head
    {
        QString fileName = m_projectPath + QDir::separator() + QString("%1.h").arg(m_className);
        writeToFile(cpp.getHeadContent(),fileName);
    }

    //cpp
    {
        QString fileName = m_projectPath + QDir::separator() + QString("%1.cpp").arg(m_className);
        writeToFile(cpp.getCppContent(),fileName);
    }
}

void GenerateProjectFile::outputMain()
{
    QString content = ConcatNewLine("#include \"" + m_className + ".h\"");
    content += ConcatNewLine("#include <QApplication>");
    content += ConcatNewLine("#include <QFile>");
    content += New_Line;
    content += ConcatNewLine("int main(int argc, char *argv[])");
    content += ConcatNewLine("{");
    content += ConcatNewLine("    QApplication a(argc, argv);");
    content += New_Line;
    content += ConcatNewLine("    QFile f(\":/style/" + m_qssFileName + "\");");
    content += ConcatNewLine("    if(f.open(QFile::ReadOnly)){");
    content += ConcatNewLine("        a.setStyleSheet(f.readAll());");
    content += ConcatNewLine("    }");
    content += New_Line;
    content += ConcatNewLine("    " + m_className + " w;");
    content += ConcatNewLine("    w.showMaximized();");
    content += New_Line;
    content += ConcatNewLine("    return a.exec();");
    content += ConcatNewLine("}");

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
