#include "generateprojectfile.h"

#include <QFile>
#include <QDir>
#include <QDebug>
#include "../util/rutil.h"
#include "cppgenerate.h"
#include "code/modelswitchtemplate.h"
#include "code/playcontroltemplate.h"
#include "code/pageswitchtemplate.h"
#include "code/twswitchtemplate.h"
#include "code/mutexbuttontemplate.h"
#include "code/tablewidgetstyletemplate.h"
#include "code/dropdownbutton.h"
#include "code/custominputbox.h"
#include "code/customscrollbar.h"
#include "code/myfloatingwindow.h"
#include "code/customfoldingcontrol.h"
#include "code/customswichbtn.h"
#include "code/dropdownbox.h"
#include "code/switchpulldownbtn.h"
#include "code/buttongroupplate.h"

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

void GenerateProjectFile::setWindMinimumSize(QSize miniSize)
{
    m_minimumSize = miniSize;
}
void GenerateProjectFile::setOutputInfo(QString path, QString className,QString qssFileName)
{
    m_projectPath = path;
    m_className = className;
    m_qssFileName = qssFileName;
}

/**
 * @brief 根据已经提升的类名拷贝对应的类
 * @param path自定义类路径
 * @param customClassList类名列表
 */
void GenerateProjectFile::setCustomControlPath(QString path, QStringList customClassList)
{
    QString rootPath = "CustomControl";

    QString customControlPath = path + QDir::separator() + rootPath;

    m_customCppNames.clear();
    m_customHeadNames.clear();

    for(QString className : customClassList){
        QString basePath = customControlPath + QDir::separator() + className;

        QDir baseDir(basePath);
        if(baseDir.exists()){
            QFileInfoList srcFileInfos = baseDir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot);
            std::for_each(srcFileInfos.begin(),srcFileInfos.end(),[&](QFileInfo & fileInfo){

                if(fileInfo.fileName().contains(QString(".h"))){
                    if(!m_customHeadNames.contains(fileInfo.fileName()))
                        m_customHeadNames.append(fileInfo.fileName());
                }
                else if(fileInfo.fileName().contains(".cpp")){

                    if(!m_customCppNames.contains(fileInfo.fileName()))
                        m_customCppNames.append(fileInfo.fileName());
                }
                QString dstImageFullPath = m_projectPath + QDir::separator() + fileInfo.fileName();
                QFile::copy(fileInfo.filePath(),dstImageFullPath);
            });
        }
    }

//    QDir customControlDir(customControlPath);

}

void GenerateProjectFile::startOutput(bool generateCode)
{
    outputProFile();
    outputCpp(generateCode);
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
    content += QString("           " + m_className+".cpp");
    content += addCustomControlCppFile();
    content += New_Line;
    content += QString("HEADERS  += " + m_className+".h");
    content += addCustomControlHeadFile();
    content += New_Line;
    content += ConcatNewLine("FORMS    += " + m_className+".ui");
    content += New_Line;
    content += ConcatNewLine("RESOURCES += res.qrc ");

    QString fileName = m_projectPath + QDir::separator() + QString("%1.pro").arg(m_className);
    writeToFile(content,fileName);
}

QString GenerateProjectFile::addCustomControlCppFile()
{
    QString content;
    for(QString fileName:m_customCppNames){
        content += ConcatNewLine("\\ ");
        content += QString("           " + fileName);
    }
    return content;
}

QString GenerateProjectFile::addCustomControlHeadFile()
{
    QString content;
    for(QString fileName:m_customHeadNames){
        content += ConcatNewLine("\\ ");
        content += QString("           " + fileName);
    }
    return content;
}
void GenerateProjectFile::outputCpp(bool generateCode)
{
    CXX::CppGenerate cpp;
    cpp.setClssName(m_className);
    cpp.setWindoMiniSize(m_minimumSize);


    QMap<CXX::CodeType,int> t_typeStatics;

    //生成插件对应的代码
    foreach (CXX::AbstractCppCodeData * codeData, m_codeDatas) {

        if(t_typeStatics.contains(codeData->m_type)){
            t_typeStatics.operator [](codeData->m_type)++;
        }else{
            t_typeStatics.insert(codeData->m_type,1);
        }

        int typeIndex = t_typeStatics.value(codeData->m_type);


        if(generateCode){
            switch(codeData->m_type){
                case CXX::MODEL_SWITCH:{
                    CXX::ModelSwitchCodeData * switchData = dynamic_cast<CXX::ModelSwitchCodeData *>(codeData);

                    CXX::ModelSwitchTemplate modelSwitch;
                    modelSwitch.setSameTypeIndex(typeIndex);
                    modelSwitch.setModelIds(switchData->m_modelIds);
                    modelSwitch.prepareOutput(&cpp);
                }
                    break;

                case CXX::TW_SWITCH:{
                    CXX::TWSwitchCodeData * twSwitchData = dynamic_cast<CXX::TWSwitchCodeData *>(codeData);

                    CXX::TwSwitchTemplate twSwitch;
                    twSwitch.setSameTypeIndex(typeIndex);
                    twSwitch.setIds(twSwitchData->m_twPopButtId,twSwitchData->m_twContainerId,twSwitchData->m_buttContainerId,twSwitchData->m_buttIds);
                    twSwitch.prepareOutput(&cpp);
                }
                    break;

                case CXX::PAGE_SWITCH:{
                    CXX::PageSwitchCodeData * switchData = dynamic_cast<CXX::PageSwitchCodeData *>(codeData);

                    CXX::PageSwitchTemplate pageSwitch;
                    pageSwitch.setSameTypeIndex(typeIndex);
                    pageSwitch.setIds(switchData->m_leftPage,switchData->m_rightPage,switchData->m_pageIds);
                    pageSwitch.prepareOutput(&cpp);
                }
                    break;

                case CXX::PLAY_CONTROL:{
                    CXX::PlayControlCodeData * playData = dynamic_cast<CXX::PlayControlCodeData *>(codeData);

                    CXX::PlayControlTemplate playControl;
                    playControl.setSameTypeIndex(typeIndex);
                    playControl.setStackedId(playData->m_stackedWidgetId);
                    playControl.setModelIds(playData->m_modelIds);
                    playControl.prepareOutput(&cpp);
                }
                    break;

                case CXX::MUTEX_BUTTON:{
                    CXX::MutexButtonCodeData * mutexData = dynamic_cast<CXX::MutexButtonCodeData *>(codeData);

                    CXX::MutexButtonTemplate mutexButton;
                    mutexButton.setSameTypeIndex(typeIndex);
                    mutexButton.setMutexButtonIds(mutexData->m_buttIds);
                    mutexButton.prepareOutput(&cpp);

                    break;
                }
                case CXX::CUSTOM_DROP_BUTTON:{
                    CXX::DropDownButtonData * dropBtnData = dynamic_cast<CXX::DropDownButtonData *>(codeData);

                    CXX::DropdownButton dropDownButton;

                    dropDownButton.setCustomButtonStyle(dropBtnData);

                    dropDownButton.prepareOutput(&cpp);

                    break;

                }case CXX::CUSTOM_KEYBOARD_RFIELD:{
                    CXX::KeyBoardInputBoxData * customInputBoxData = dynamic_cast<CXX::KeyBoardInputBoxData *>(codeData);

                    CXX::CustomInputBox customInputBox;
                    customInputBox.setSameTypeIndex(typeIndex);

                    customInputBox.setInputBoxParameter(customInputBoxData);//获取输入框的样式参数

                    customInputBox.prepareOutput(&cpp);
                }
                    break;
                case CXX::CUSTOM_BIDIRECTIONAL_SLIDER:{

                    CXX::BidirectionalSlider * scrollBar = dynamic_cast<CXX::BidirectionalSlider *>(codeData);

                    CXX::CustomScrollBar scrollBarData;
                    scrollBarData.setSameTypeIndex(typeIndex);

                    scrollBarData.setScrollBarParameter(scrollBar);//获取输入框的样式参数

                    scrollBarData.prepareOutput(&cpp);
                    break;
                }
                case CXX::CUSTOM_FLOATING_WINDOW:{

                    CXX::FloatingWindow * floatingWindow = dynamic_cast<CXX::FloatingWindow *>(codeData);

                    CXX::MyFloatingWindow floatingWindowData;
                    floatingWindowData.setSameTypeIndex(typeIndex);

                    floatingWindowData.setFloatingWindowParameter(floatingWindow);//获取输入框的样式参数

                    floatingWindowData.prepareOutput(&cpp);
                    break;
                }
                case CXX::CUSTOM_FOLDING_CONTROL:{

                    CXX::FoldingControls * foldingControls = dynamic_cast<CXX::FoldingControls *>(codeData);

                    CXX::CustomFoldingControl foldingControlData;
                    foldingControlData.setSameTypeIndex(typeIndex);

                    foldingControlData.setFoldingControlParameter(foldingControls);//获取输入框的样式参数

                    foldingControlData.prepareOutput(&cpp);
                    break;
                }
                case CXX::CUSTOM_SWITCH_BUTTON:{

                    CXX::customSwitchButton * customSwitchBtn = dynamic_cast<CXX::customSwitchButton *>(codeData);

                    CXX::CustomSwichBtn switchBtnData;
                    switchBtnData.setSameTypeIndex(typeIndex);

                    switchBtnData.setCustomSwitchBtnData(customSwitchBtn);//获取输入框的样式参数

                    switchBtnData.prepareOutput(&cpp);
                    break;
                }
            case CXX::CUSTOM_DROPDOWN_BOX :{

                CXX::DropDownBoxData * dropDownBoxData = dynamic_cast<CXX::DropDownBoxData *>(codeData);

                CXX::DropDownBox dropDownBox;
                dropDownBox.setSameTypeIndex(typeIndex);

                dropDownBox.setIds(dropDownBoxData->m_widgetID,dropDownBoxData->m_buttonID,dropDownBoxData->m_optionIdList);

                dropDownBox.prepareOutput(&cpp);

                break;
            }
            case CXX::CUSTOM_SWITCH_PULLDOWN_BUTTON:{
                CXX::SwitchPullDownButtonData * switchPullDownBtnData = dynamic_cast<CXX::SwitchPullDownButtonData *>(codeData);

                CXX::SwitchPullDownBtn switchPullDownBtnCode;

                switchPullDownBtnCode.setSameTypeIndex(typeIndex);


                switchPullDownBtnCode.setIds(switchPullDownBtnData);

                switchPullDownBtnCode.prepareOutput(&cpp);

                break;
            }
            case CXX::CUSTOM_BUTTON_GROUP:{
                CXX::ButtonGroupCodeData * btnGroupData = dynamic_cast<CXX::ButtonGroupCodeData *>(codeData);

                CXX::ButtonGroupPlate buttonGroupPlate;

                buttonGroupPlate.setSameTypeIndex(typeIndex);


                buttonGroupPlate.setButtonGroupIds(btnGroupData->m_buttIds);

                buttonGroupPlate.prepareOutput(&cpp);

                break;
            }
                default:break;
            }
        }

        //表格默认使用代码生成
        if(codeData->m_type == CXX::CUSTOM_TABLE_WIDGET){
            CXX::TableStyleCodeData * tableData = dynamic_cast<CXX::TableStyleCodeData *>(codeData);

            CXX::TableWidgetStyleTemplate tableStyle;
            tableStyle.setSameTypeIndex(typeIndex);
            tableStyle.setTableStyleData(tableData);
            tableStyle.prepareOutput(&cpp);
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
        writeToBomFile(cpp.getCppContent(),fileName);
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

void GenerateProjectFile::writeToFile(QString content, QString fileName)
{
    QFile proFile(fileName);
    if(proFile.open(QFile::WriteOnly)){
        proFile.write(content.toLocal8Bit());
    }
}

void GenerateProjectFile::writeToBomFile(QString content, QString fileName)
{
    QFile proFile(fileName);
    if(proFile.open(QFile::WriteOnly)){
        QTextStream fileOut(&proFile);
        fileOut.setCodec("utf-8");
        fileOut.setGenerateByteOrderMark(true);
        fileOut<<content.toLocal8Bit();
    }
}
