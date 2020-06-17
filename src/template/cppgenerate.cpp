#include "cppgenerate.h"

#include <QDebug>

extern QString New_Line;
extern QString New_Space;
extern QString ConcatNewLine(QString text);
extern QString ConcatTabSpace(QString text);

namespace CXX{

CppGenerate::CppGenerate()
{

}

QString CppGenerate::getHeadContent()
{
    QString upperClss = m_clazzName.toUpper();
    QString content = ConcatNewLine("#ifndef " + upperClss + "_H");
    content += ConcatNewLine("#define " + upperClss + "_H");
    content += New_Line;
    content += ConcatNewLine("#include <QWidget>");

    if(m_includes.size() > 0){
        foreach (QString tinclude, m_includes) {
            content += ConcatNewLine(tinclude);
        }
    }

    content += New_Line;
    content += ConcatNewLine("namespace Ui {");
    content += ConcatNewLine("class " + m_clazzName + ";");
    content += ConcatNewLine("}");
    content += New_Line;
    content += ConcatNewLine("class " + m_clazzName + " : public QWidget");
    content += ConcatNewLine("{");
    content += ConcatNewLine("    Q_OBJECT");
    content += New_Line;
    content += ConcatNewLine("public:");
    content += ConcatNewLine("    explicit " + m_clazzName + "(QWidget *parent = 0);");
    content += ConcatNewLine("    ~" + m_clazzName + "();");

    content += New_Line;

    if(m_privateSlots.size() > 0){
        content += ConcatNewLine("private slots:");
        foreach (QString tslots, m_privateSlots) {
            content += ConcatTabSpace(ConcatNewLine(tslots));
        }
        content += New_Line;
    }

    if(m_privateFuns.size() > 0){
        content += ConcatNewLine("private:");
        foreach (QString tfunc, m_privateFuns) {
            content += ConcatTabSpace(ConcatNewLine(tfunc));
        }
        content += New_Line;
    }

    content += New_Line;
    content += ConcatNewLine("private:");
    content += ConcatNewLine("    Ui::" + m_clazzName + " *ui;");

    if(m_privateMembers.size() > 0){
        foreach (QString member, m_privateMembers) {
            content += ConcatTabSpace(ConcatNewLine(member));
        }
        content += New_Line;
    }

    content += ConcatNewLine("};");
    content += New_Line;
    content += ConcatNewLine("#endif // " + upperClss + "_H");

    return content;
}

QString CppGenerate::getCppContent()
{
    QString content = ConcatNewLine("#include \""+m_clazzName+".h\"");
    content += ConcatNewLine("#include \"ui_"+m_clazzName+".h\"");
    content += New_Line;
    content += ConcatNewLine(m_clazzName+"::"+m_clazzName+"(QWidget *parent) :");
    content += ConcatNewLine("    QWidget(parent),");
    content += ConcatNewLine("    ui(new Ui::"+m_clazzName+")");
    content += ConcatNewLine("{");
    content += ConcatNewLine("    ui->setupUi(this);");

    if(m_constructInitCode.size() > 0){
        foreach (QString code, m_constructInitCode) {
            content += ConcatTabSpace(ConcatNewLine(code));
        }
    }

    content += ConcatNewLine("}");
    content += New_Line;

    //析构函数
    content += ConcatNewLine(m_clazzName+"::~"+m_clazzName+"()");
    content += ConcatNewLine("{");
    content += ConcatNewLine("    delete ui;");
    content += ConcatNewLine("}");

    for(QString funcImp : m_funImplements){
        content += New_Line;
        content += ConcatNewLine(funcImp);
    }

    content += New_Line;

    return content;
}

};
