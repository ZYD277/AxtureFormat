#include "modelswitchtemplate.h"

#include "../cppgenerate.h"

namespace CXX{

ModelSwitchTemplate::ModelSwitchTemplate():AbstractCodeTemplate(MODEL_SWITCH)
{

}

void ModelSwitchTemplate::prepareOutput(CppGenerate *generate)
{
    QString t_slotName = QString("switchState_%1(bool flag)").arg(m_sameTypeIndex);
    generate->addPrivateSlot(QString("void %1;").arg(t_slotName));

    QString t_baseMemberName = QString("m_butts_%1").arg(m_sameTypeIndex);
    generate->addPrivateMember(QString("QList<QPushButton *> %1;").arg(t_baseMemberName));

    generate->addInclude("#include <QPushButton>");

    for(QString id:m_modelIds){
        generate->addConstructInitCode(QString("connect(ui->%1,SIGNAL(clicked(bool)),this,SLOT(%2));")
                                       .arg(id)
                                       .arg(QString("switchState_%1(bool)").arg(m_sameTypeIndex)));
    }

    generate->addConstructInitCode(NEW_EMPTY);

    for(QString id:m_modelIds){
        generate->addConstructInitCode(QString("%1.append(ui->%2);").arg(t_baseMemberName).arg(id));
    }

    generate->addConstructInitCode(NEW_EMPTY);

    QString code = ConcatNewLine(QString("void %1::%2").arg(generate->getClassName()).arg(t_slotName));
    code += ConcatNewLine("{");
    code += ConcatNewLine("    QPushButton * tsend = dynamic_cast<QPushButton*>(QObject::sender());");
    code += ConcatNewLine("    for(QPushButton * butt : "+t_baseMemberName+"){");
    code += ConcatNewLine("        if(butt != tsend){");
    code += ConcatNewLine("            butt->setChecked(false);");
    code += ConcatNewLine("        }else{");
    code += ConcatNewLine("            butt->setChecked(true);");
    code += ConcatNewLine("        }");
    code += ConcatNewLine("    }");
    code += ConcatNewLine("}");

    generate->addFunImplement(code);
}

} //namespace CXX
