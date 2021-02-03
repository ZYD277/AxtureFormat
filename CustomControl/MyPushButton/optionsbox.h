#ifndef OPTIONSBOX_H
#define OPTIONSBOX_H

#include <QWidget>
#include <QPushButton>
#include <QList>
#include <QLabel>
#include <QPropertyAnimation>
#include <QLineEdit>
#include "head.h"

class OptionsBox : public QWidget
{
    Q_OBJECT
public:
    explicit OptionsBox(QWidget *parent = 0);
    ~OptionsBox();

    void setTitletype(PopupWindowType type);

    void attachTarget(QWidget *widget);

    void showPopup();

    void hidePopup();

    //通过结构体获取样式
    void setControlStyle(VirtualKeyboard keyBoardStyle);
    void setTitleLaout();
    void setKeyBoardLaout();
    void setTailLaout();

    /////////
    void setOptionsBoxInfo(OptionPopUp optionsBoxInfo);

signals:
    void setCurrentText(QString);

public slots:
    void buttonText();

    void getEditBoxText(QString editBoxText);

    void setThisFocus();

private:
    void initView();

    void setSrcImg(QString controlID, QString srcImg);

    //////////////////

    void setTitleInfo(QWidget *parentWidget, TitleInfo titleInfo, Location pLocation);
    void createBtnGroup(QWidget *parentWidget, ButtonGroup btnGroupInfo, bool createBtnChecked);

    void changeCurrentText(QString btnText);
    QRect getControlLocation(Location plocation,Location slocation);
    QString setControlStyleSheet(QString srcImage);

    void setBtnGroup(QList<BUTTON> buttons, QWidget *parentWidget, Location pLocation);

private:

    //////////
    OptionPopUp m_optionsBoxInfo;        /*!< 选项框信息 */


    //绑定下拉框主体，用于计算弹出位置
    QWidget *targetWidget;
    //弹出选项框
    QPropertyAnimation *animation;

    QWidget * m_mainWidget;
    QWidget * m_optionsWidget;           /*!< optionsBox */
    QWidget * m_titleWidget;            /*!< 选项框标题 */
    QWidget * m_functionBtnWidget;      /*!< 功能按钮区 */

    QString m_checkedText;          /*!< 选中文本 */

    PopupWindowType m_titleType;             /*!< 标题区控件类型 */

    int m_boxMaxwidth;   /*!< 选项框最大宽度 */
    int m_boxMaxHeith;   /*!< 选项框最大高度 */

    //获取自定义输入框的的样式
    VirtualKeyboard m_keyBoardStyle;
    QLineEdit *m_titleLine; //标题为文本输入框时使用
    QString m_editBoxText;       /*!< 触发编辑框的文本 */
};

#endif // OPTIONSBOX_H
