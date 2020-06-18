#include "pageswitchtemplate.h"

#include "../cppgenerate.h"

namespace CXX{

PageSwitchTemplate::PageSwitchTemplate():AbstractCodeTemplate(PAGE_SWITCH)
{

}

void PageSwitchTemplate::prepareOutput(CppGenerate *generate)
{
    QString t_slotName = QString("switchPage_%1(bool flag)").arg(m_sameTypeIndex);
    generate->addPrivateSlot(QString("void %1;").arg(t_slotName));

    QString t_pageSlotName = QString("pageSwitch_%1()").arg(m_sameTypeIndex);
    generate->addPrivateSlot(QString("void %1;").arg(t_pageSlotName));

    generate->addInclude("#include <QPushButton>");
    generate->addInclude("#include <QList>");

    QString t_baseMemberName = QString("m_pageButts_%1").arg(m_sameTypeIndex);
    generate->addPrivateMember(QString("QList<QPushButton *> %1;").arg(t_baseMemberName));

    for(QString id:m_pageIds){
        generate->addConstructInitCode(QString("connect(ui->%1,SIGNAL(clicked(bool)),this,SLOT(%2));")
                                       .arg(id)
                                       .arg(QString("switchPage_%1(bool)").arg(m_sameTypeIndex)));
    }

    generate->addConstructInitCode(NEW_EMPTY);

    generate->addConstructInitCode(QString("connect(ui->%1,SIGNAL(pressed()),this,SLOT(%2));").arg(m_leftPageId).arg(t_pageSlotName));
    generate->addConstructInitCode(QString("connect(ui->%1,SIGNAL(pressed()),this,SLOT(%2));").arg(m_rightPageId).arg(t_pageSlotName));
    generate->addConstructInitCode(QString("ui->%1->setProperty(\"direction\",\"left\");").arg(m_leftPageId));
    generate->addConstructInitCode(QString("ui->%1->setProperty(\"direction\",\"right\");").arg(m_rightPageId));

    generate->addConstructInitCode(NEW_EMPTY);

    for(QString id:m_pageIds){
        generate->addConstructInitCode(QString("%1.append(ui->%2);").arg(t_baseMemberName).arg(id));
    }

    generate->addConstructInitCode(NEW_EMPTY);

    QString code = ConcatNewLine(QString("void %1::%2").arg(generate->getClassName()).arg(t_slotName));
    code += ConcatNewLine("{");
    code += ConcatNewLine("    QPushButton * tsend = dynamic_cast<QPushButton*>(QObject::sender());");
    code += ConcatNewLine("    for(QPushButton * butt : " + t_baseMemberName + "){");
    code += ConcatNewLine("        if(butt != tsend){");
    code += ConcatNewLine("            butt->setChecked(false);");
    code += ConcatNewLine("        }else{");
    code += ConcatNewLine("            butt->setChecked(true);");
    code += ConcatNewLine("        }");
    code += ConcatNewLine("    }");
    code += ConcatNewLine("}");

    generate->addFunImplement(code);

    code = ConcatNewLine(QString("void %1::%2").arg(generate->getClassName()).arg(t_pageSlotName));
    code += ConcatNewLine("{");
    code += ConcatNewLine("    bool t_left =  true;");
    code += ConcatNewLine("    if(QObject::sender()->property(\"direction\").toString().contains(\"right\"))");
    code += ConcatNewLine("        t_left = false;");
    code += ConcatNewLine(NEW_EMPTY);
    code += ConcatNewLine("    int index = 0;");
    code += ConcatNewLine("    for(int i = 0; i < %1.size(); i++){").arg(t_baseMemberName);
    code += ConcatNewLine("        if(%1.at(i)->isChecked()){").arg(t_baseMemberName);
    code += ConcatNewLine("            index = i;");
    code += ConcatNewLine("            break;");
    code += ConcatNewLine("        }");
    code += ConcatNewLine("    }");
    code += ConcatNewLine(NEW_EMPTY);
    code += ConcatNewLine("    if(t_left){");
    code += ConcatNewLine("        index = index > 0 ? index - 1: 0;");
    code += ConcatNewLine("    }else{");
    code += ConcatNewLine("        index = index < %1.size() - 1 ? index + 1: %2.size() - 1;").arg(t_baseMemberName).arg(t_baseMemberName);
    code += ConcatNewLine("    }");
    code += ConcatNewLine(NEW_EMPTY);
    code += ConcatNewLine("    for(int i = 0; i < %1.size(); i++){").arg(t_baseMemberName);
    code += ConcatNewLine("        if(i == index){");
    code += ConcatNewLine("            %1.at(i)->setChecked(true);").arg(t_baseMemberName);
    code += ConcatNewLine("        }else{");
    code += ConcatNewLine("            %1.at(i)->setChecked(false);").arg(t_baseMemberName);
    code += ConcatNewLine("        }");
    code += ConcatNewLine("    }");
    code += ConcatNewLine("}");

    generate->addFunImplement(code);

}

void PageSwitchTemplate::setIds(QString leftId, QString rightId, QStringList pageIds)
{
    m_leftPageId = leftId;
    m_rightPageId = rightId;
    m_pageIds = pageIds;
}

} //namespace CXX
