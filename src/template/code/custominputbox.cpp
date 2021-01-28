#include "custominputbox.h"

#include "../cppgenerate.h"

namespace CXX{

CustomInputBox::CustomInputBox() : AbstractCodeTemplate(CUSTOM_KEYBOARD_RFIELD)
{

}

void CustomInputBox::prepareOutput(CppGenerate *generate)
{
    //弹出框部分
	QString t_baseName = QString("t_BassAttribute_%1").arg(m_sameTypeIndex);
	QString t_locationName = QString("t_location_%1").arg(m_sameTypeIndex);
	QString t_buttonName = QString("t_buttonAttribute_%1").arg(m_sameTypeIndex);
	QString t_triggerName = QString("t_triggerInputBox_%1").arg(m_sameTypeIndex);
	QString t_titleName = QString("t_title_%1").arg(m_sameTypeIndex);
	QString t_theKeysName = QString("t_theKeys_%1").arg(m_sameTypeIndex);
	QString t_tailGroupName = QString("t_tail_%1").arg(m_sameTypeIndex);
	QString t_virtualKeyboardName = QString("t_virtualKeyboard_%1").arg(m_sameTypeIndex);
	QString t_overallStructureName = QString("m_inputBoxData_%1").arg(m_sameTypeIndex);

    QString t_iconInfoName = QString("m_iconInfo_%1").arg(m_sameTypeIndex);

    QString t_iconInfo = QString("         IconInfo %1;").arg(t_iconInfoName);
	QString t_baseGroup = QString("         BassAttribute %1;").arg(t_baseName);
	QString t_locationGroup = QString("         Location %1;").arg(t_locationName);
	QString t_buttonGroup = QString("         BUTTON %1;").arg(t_buttonName);
	QString t_triggerGroup = QString("         TriggerInputBox %1;").arg(t_triggerName);
	QString t_titleGroup = QString("         TITLE %1;").arg(t_titleName);
	QString t_theKeysGroup = QString("         THEKEYS %1;").arg(t_theKeysName);
	QString t_tailGroup = QString("         TAIL %1;").arg(t_tailGroupName);
	QString t_virtualKeyboardGroup = QString("         VirtualKeyboard %1;").arg(t_virtualKeyboardName);
	QString t_overallStructure = QString("         ControlStyles %1;").arg(t_overallStructureName);

	QString customStyle;
    customStyle += t_baseGroup;
    customStyle += ConcatNewLine(t_locationGroup);
    customStyle += ConcatNewLine(t_iconInfo);
    customStyle += ConcatNewLine(t_buttonGroup);
    customStyle += ConcatNewLine(t_triggerGroup);
    customStyle += ConcatNewLine(t_titleGroup);
    customStyle += ConcatNewLine(t_theKeysGroup);
    customStyle += ConcatNewLine(t_tailGroup);
    customStyle += ConcatNewLine(t_virtualKeyboardGroup);
    customStyle += ConcatNewLine(t_overallStructure);
	customStyle += ConcatNewLine(NEW_EMPTY);


	auto getControlLocation = [&](QString groupName, Location location) {

		QString controlLocation;
		controlLocation += QString("         %1.m_height = %2;").arg(t_locationName).arg(location.m_height);
		controlLocation += ConcatNewLine(NEW_EMPTY);
		controlLocation += QString("         %1.m_width = %2;").arg(t_locationName).arg(location.m_width);
		controlLocation += ConcatNewLine(NEW_EMPTY);
		controlLocation += QString("         %1.m_top = %2;").arg(t_locationName).arg(location.m_top);
		controlLocation += ConcatNewLine(NEW_EMPTY);
		controlLocation += QString("         %1.m_left = %2;").arg(t_locationName).arg(location.m_left);
		controlLocation += ConcatNewLine(NEW_EMPTY);

		controlLocation += QString("         %1.m_location = %2;").arg(groupName).arg(t_locationName);
		controlLocation += ConcatNewLine(NEW_EMPTY);

		return controlLocation;
	};

    auto getControlBaseInfo = [&](QString groupName,BassAttribute &baseInfo){

        customStyle += getControlLocation(t_baseName,baseInfo.m_location);

        QString controlBaseInfo;
        controlBaseInfo += ConcatNewLine(QString("         %1.m_text = QString::fromLocal8Bit(\"%2\");").arg(t_baseName).arg(baseInfo.m_text));
        controlBaseInfo += ConcatNewLine(QString("         %1.m_srcImage = QString::fromLocal8Bit(\"%2\");").arg(t_baseName).arg(baseInfo.m_srcImage));
        controlBaseInfo += ConcatNewLine(QString("         %1.m_ID = QString::fromLocal8Bit(\"%2\");").arg(t_baseName).arg(baseInfo.m_ID));


        customStyle += getControlLocation(t_iconInfoName,baseInfo.m_iconInfo.m_location);
        controlBaseInfo += ConcatNewLine(QString("         %1.m_srcIcon = QString::fromLocal8Bit(\"%2\");").arg(t_iconInfoName).arg(baseInfo.m_iconInfo.m_srcIcon));

        controlBaseInfo += ConcatNewLine(QString("         %1.m_ID = QString::fromLocal8Bit(\"%2\");").arg(t_iconInfoName).arg(baseInfo.m_iconInfo.m_ID));

        controlBaseInfo += ConcatNewLine(QString("         %1.m_iconInfo = %2;").arg(t_baseName).arg(t_iconInfoName));

        controlBaseInfo += ConcatNewLine(QString("         %1.m_baseAtrribute = %2;").arg(groupName).arg(t_baseName));

        return controlBaseInfo;
    };

    //获取触发输入框的基本信息
    customStyle += ConcatNewLine(NEW_EMPTY);
    customStyle += getControlBaseInfo(t_triggerName,m_inputBoxData->m_triggerInputBox.m_baseAtrribute);
    customStyle += ConcatNewLine(NEW_EMPTY);
    customStyle += getControlLocation(t_triggerName,m_inputBoxData->m_triggerInputBox.m_location);
    customStyle += ConcatNewLine(NEW_EMPTY);

    //虚拟键盘框

    //获取虚拟键盘的位置和基本信息    
    customStyle += getControlBaseInfo(t_virtualKeyboardName,m_inputBoxData->m_virtualKeyGroup.m_baseAtrribute);
    customStyle += getControlLocation(t_virtualKeyboardName,m_inputBoxData->m_virtualKeyGroup.m_location);

    //获取虚拟键盘的head部分位置以及基本信息

    customStyle += getControlBaseInfo(t_titleName,m_inputBoxData->m_virtualKeyGroup.m_title.m_baseAtrribute);
    customStyle += getControlLocation(t_titleName,m_inputBoxData->m_virtualKeyGroup.m_title.m_location);

    //获取虚拟键盘的按键区的基本信息和位置

    customStyle += getControlBaseInfo(t_theKeysName,m_inputBoxData->m_virtualKeyGroup.m_keyGroup.m_baseAtrribute);
    customStyle += getControlLocation(t_theKeysName,m_inputBoxData->m_virtualKeyGroup.m_keyGroup.m_location);

    //获取虚拟键盘的退出按钮区位置和基本信息

    customStyle += getControlBaseInfo(t_tailGroupName,m_inputBoxData->m_virtualKeyGroup.m_tailGroup.m_baseAtrribute);
    customStyle += getControlLocation(t_tailGroupName,m_inputBoxData->m_virtualKeyGroup.m_tailGroup.m_location);

    //获取虚拟键盘每个按键的样式

    for(BUTTON t_buttonInfo : m_inputBoxData->m_virtualKeyGroup.m_keyGroup.m_keyBtns){
       customStyle += ConcatNewLine(QString("         %1.deleteBtn = %2;").arg(t_buttonName).arg(t_buttonInfo.deleteBtn));
       customStyle += ConcatNewLine(getControlBaseInfo(t_buttonName,t_buttonInfo.m_baseAtrribute));
       customStyle += ConcatNewLine(getControlLocation(t_buttonName,t_buttonInfo.m_location));
       customStyle += ConcatNewLine(QString("         %1.m_keyBtns.append(%2);").arg(t_theKeysName).arg(t_buttonName));
    }

    for(BUTTON t_buttonInfo : m_inputBoxData->m_virtualKeyGroup.m_tailGroup.m_quitBtns){
       customStyle += ConcatNewLine(getControlBaseInfo(t_buttonName,t_buttonInfo.m_baseAtrribute));
       customStyle += ConcatNewLine(getControlLocation(t_buttonName,t_buttonInfo.m_location));
       customStyle += ConcatNewLine(QString("         %1.m_quitBtns.append(%2);").arg(t_tailGroupName).arg(t_buttonName));
    }

    customStyle += ConcatNewLine(QString("         %1.m_ID = QString(\"%2\");").arg(t_virtualKeyboardName).arg(m_inputBoxData->m_virtualKeyGroup.m_ID));

    customStyle += ConcatNewLine(QString("         %1.m_title = %2;").arg(t_virtualKeyboardName).arg(t_titleName));
    customStyle += ConcatNewLine(QString("         %1.m_keyGroup = %2;").arg(t_virtualKeyboardName).arg(t_theKeysName));
    customStyle += ConcatNewLine(NEW_EMPTY);
    customStyle += ConcatNewLine(QString("         %1.m_tailGroup = %2;").arg(t_virtualKeyboardName).arg(t_tailGroupName));
    customStyle += ConcatNewLine(NEW_EMPTY);

    customStyle += ConcatNewLine(QString("         %1.m_virtualKeyGroup = %2;").arg(t_overallStructureName).arg(t_virtualKeyboardName));
    customStyle += ConcatNewLine(QString("         %1.m_triggerInputBox = %2;").arg(t_overallStructureName).arg(t_triggerName));
    customStyle += ConcatNewLine(NEW_EMPTY);


    QString code;
    code += ConcatNewLine("{");
    code += ConcatNewLine(customStyle);
    code += ConcatNewLine("         ui->%1->setControlStyle(%2);").arg(m_inputBoxData->m_inputBoxId).arg(t_overallStructureName);
    code += ConcatNewLine("    }");


    generate->addConstructInitCode(code);

}

void CustomInputBox::setInputBoxParameter(KeyBoardInputBoxData *inputBoxData)
{
    m_inputBoxData = inputBoxData;
}

} //namespace CXX
