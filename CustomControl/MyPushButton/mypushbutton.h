#ifndef MYPUSHBUTTON_H
#define MYPUSHBUTTON_H

#include <QWidget>
#include <QAbstractButton>
#include <QLineEdit>
#include <QString>

#include "optionsbox.h"
#include "head.h"

class MyPushButton : public QAbstractButton
{
    Q_OBJECT
public:
    explicit MyPushButton(QWidget *parent = 0);

    /**
     * @brief 图标尺寸
     */
    enum IconSize {
        ICON_16,
        ICON_24,
        ICON_32,
        ICON_48,
        ICON_64,
        ICON_custom
    };

    /**
     * @brief 控件状态
     */
    enum ControlState {
        NORMAL = 0X01,
        HOVER = 0X02,
        SELECTED = 0X04,
        PRESSED = 0x06,
        DISABLE = 0X08//禁止点击状态
    };

    /**
     * @brief 背景样式
     */
    struct BackgroundStyle{
        QString m_default;
        QString m_hover;
        QString m_pressed;
        QString m_checked;
    };

    /**
     * @brief 文字样式
     */
    struct FontStyle{
        QString m_default;
        QString m_hover;
        QString m_pressed;
        QString m_checked;
    };

    /**
     * @brief 背景图片以及图标
     */
    struct BackgroundImg{
        QPixmap m_normalPix;
        QPixmap m_hoverPix;
        QPixmap m_pressedPix;
        QPixmap m_selectedPix;
        QPixmap m_disablePix;

        QPixmap m_normalIcon;            /*!< 默认状态下图标 */
        QPixmap m_hoverIcon;             /*!< 悬浮状态下图标 */
        QPixmap m_pressIcon;            /*!< 按下状态下图标 */
        QPixmap m_selectIcon;            /*!< 选中状态下图标 */
        QPixmap m_disableIcon;           /*!< 失能状态下图标 */
    };

    void setIconSize(IconSize type, QSize size = QSize());

    void getParent(QWidget *parent);

    QSize sizeHint() const;
    QSize minimumSizeHint() const;

    //获取不同状态下的ICON
    void SetBtnState(ControlState state);

    //获取下拉按钮样式
    void setSelfStyleSheet(DropDownButtonData customBtnInfo);

signals:

protected:
    void paintEvent(QPaintEvent * event);
    void enterEvent(QEvent * event);
    void leaveEvent(QEvent * event);
//    void mouseReleaseEvent(QMouseEvent *event);

private:
    void initView();
    QSize calcMiniumSize() const;

    //绘制边框以及背景颜色
    void drawBorderBackground(QPainter *painter, QString colorName);
    //绘制背景图片
    void drawImage(QPainter *painter,QPixmap pixmap);
    //绘制背景图标
    void drawIcon(QPainter *painter, QPixmap pixmap);
    //绘制文本
    void drawText(QPainter *painter,QString colorName);

    void setOpenStateImg();
    void setCloseStateImg();
    void setOpenStateStytle();
    void setCloseStateStytle();
    QString getBackgroundStyle(QString srcStyle);

    QString firltrateStyle(QString currentStyle,QString defauleStyle);

    void updateStateImage(QString srcCurrentImg, QString srcCurentIcon, BackgroundImg &backgroundImg);
    void updateCurrentStyle(bool isOpen);

public slots:
    void changeText(QString text);

    void btnChecked(bool checked);

private:
    //设置图标
    QSize m_iconSize;
    int m_spacing;                  /*!< 图标和文字之间的间隔 */

    //获取样式用于绘制(背景颜色使用16进制)
    BackgroundStyle m_openBgStyle;
    BackgroundStyle m_closeBgStyle;
    BackgroundStyle m_currentBgStyle;

    FontStyle m_openFontStyle;
    FontStyle m_closeFontStyle;
    FontStyle m_currentFontStyle;

    //背景图片以及图标
    BackgroundImg m_closeBgImg;
    BackgroundImg m_openBgImg;
    BackgroundImg m_currentBgImg;

    //不同状态切换标志
    int m_iMask;
    ControlState m_curState;//当前状态

    //设置图标以及文字的位置
    QPoint m_startPoint;

    bool m_mouseEnter;
    //按钮点击弹出界面
    OptionsBox *m_optionDialog; /*!< 选项框 */

    bool isBtnOpenState;

    /////
    DropDownButtonData m_customBtnInfo;
};

#endif // MYPUSHBUTTON_H
