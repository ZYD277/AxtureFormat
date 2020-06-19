#include "twswitchtemplate.h"

#include "../cppgenerate.h"

namespace CXX{

TwSwitchTemplate::TwSwitchTemplate():AbstractCodeTemplate(TW_SWITCH)
{

}

void TwSwitchTemplate::prepareOutput(CppGenerate *generate)
{
    generate->addInclude("#include <QButtonGroup>");

    QString t_slotName = QString("switchTw_%1()").arg(m_sameTypeIndex);
    generate->addPrivateSlot(QString("void %1;").arg(t_slotName));

    QString t_baseMemberName = QString("m_buttGroup_%1").arg(m_sameTypeIndex);
    generate->addPrivateMember(QString("QButtonGroup * %1;").arg(t_baseMemberName));

    generate->addConstructInitCode(NEW_EMPTY);

    for(auto pair:m_buttIds){
        generate->addConstructInitCode(QString("connect(ui->%1,SIGNAL(pressed()),this,SLOT(%2));")
                                       .arg(pair.first)
                                       .arg(QString("switchTw_%1()").arg(m_sameTypeIndex)));
    }

    generate->addConstructInitCode(NEW_EMPTY);

    for(auto pair:m_buttIds){
        generate->addConstructInitCode(QString("ui->%1->setProperty(\"tw\",%2);")
                                       .arg(pair.first).arg(pair.second));
    }

    generate->addConstructInitCode(NEW_EMPTY);

    generate->addConstructInitCode(QString("%1 = new QButtonGroup();").arg(t_baseMemberName));
    for(auto pair:m_buttIds){
        generate->addConstructInitCode(QString("%1->addButton(ui->%2);").arg(t_baseMemberName).arg(pair.first));
    }

    generate->addConstructInitCode(NEW_EMPTY);

    QString code = ConcatNewLine(QString("void %1::%2").arg(generate->getClassName()).arg(t_slotName));
    code += ConcatNewLine("{");
    code += ConcatNewLine("    int twIndex = QObject::sender()->property(\"tw\").toInt();");
    code += ConcatNewLine("    ui->%1->setCurrentIndex(twIndex - 1);").arg(m_twContainerId);
    code += ConcatNewLine("    ui->%1->setVisible(false);").arg(m_buttContainerId);
    code += ConcatNewLine("    ui->%1->setChecked(false);").arg(m_twPopButtId);
    code += ConcatNewLine("}");

    generate->addFunImplement(code);

}

void TwSwitchTemplate::setIds(QString twPopButtId, QString twContainerId, QString buttContainerId, QList<QPair<QString, QString> > buttIds)
{
    m_twPopButtId = twPopButtId;
    m_twContainerId = twContainerId;
    m_buttContainerId = buttContainerId;
    m_buttIds = buttIds;
}

} //namespace CXX
