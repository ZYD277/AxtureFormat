#include "mutexbuttontemplate.h"

#include "../cppgenerate.h"

namespace CXX{

MutexButtonTemplate::MutexButtonTemplate():AbstractCodeTemplate(MUTEX_BUTTON)
{

}

void MutexButtonTemplate::prepareOutput(CppGenerate *generate)
{
    QString t_slotName = QString("mutexButton_%1(bool flag)").arg(m_sameTypeIndex);
    generate->addPrivateSlot(QString("void %1;").arg(t_slotName));

    QString t_baseMemberName = QString("m_mutexButts_%1").arg(m_sameTypeIndex);
    generate->addPrivateMember(QString("QList<QPushButton *> %1;").arg(t_baseMemberName));

    generate->addInclude("#include <QPushButton>");

    for(QString id:m_mutexButtIds){
        generate->addConstructInitCode(QString("connect(ui->%1,SIGNAL(clicked(bool)),this,SLOT(%2));")
                                       .arg(id)
                                       .arg(QString("mutexButton_%1(bool)").arg(m_sameTypeIndex)));
    }

    generate->addConstructInitCode(NEW_EMPTY);

    for(QString id:m_mutexButtIds){
        generate->addConstructInitCode(QString("%1.append(ui->%2);").arg(t_baseMemberName).arg(id));
    }

    generate->addConstructInitCode(NEW_EMPTY);

    QString code = ConcatNewLine(QString("void %1::%2").arg(generate->getClassName()).arg(t_slotName));
    code += ConcatNewLine("{");
    code += ConcatNewLine("    QPushButton * senderButt = dynamic_cast<QPushButton*>(QObject::sender());");
    code += ConcatNewLine("    for(int i = 0; i < %1.size(); i++){").arg(t_baseMemberName);
    code += ConcatNewLine("        QPushButton * butt = %1.at(i);").arg(t_baseMemberName);
    code += ConcatNewLine("        if(butt != senderButt)");
    code += ConcatNewLine("            butt->setDisabled(flag);");
    code += ConcatNewLine("    }");
    code += ConcatNewLine("}");

    generate->addFunImplement(code);

}

void MutexButtonTemplate::setMutexButtonIds(QStringList mutexButts)
{
    m_mutexButtIds = mutexButts;
}

} //namespace CXX
