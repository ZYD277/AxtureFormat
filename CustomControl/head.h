#ifndef HEAD_H_2021
#define HEAD_H_2021

#include <QString>
/**
 * @brief 标题区类型
 */
enum PopupWindowType {
    M_LABEL,      /*!< 标签 */
    M_LINEEDIT    /*!< 输入框 */
};

////////////////////////////////////////////

/**
 * @brief 各部分基础属性
 */
struct Location{
    int m_width;
    int m_height;
    int m_top;
    int m_left;
};

/**
 * @brief 部分控件包含图标
 */
struct IconInfo{
    Location m_location;
    QString m_srcIcon;
    QString m_ID;
};

struct TextInfo{
   QString m_text;
   QString m_color;
   QString m_font;
};

struct BaseInfo{
    Location m_location;
    TextInfo m_textInfo;
    QString m_srcImg;
    QString m_style;
    QString m_className;
    QString m_dataLabel;
    QString m_ID;
};
///////////////////自定义下拉按钮////////////////////////////////
/**
 * @brief 按钮信息
 */
struct ButtonInfo{
    ButtonInfo():deleteBtn(false){}
    BaseInfo m_baseInfo;
    bool deleteBtn;//用于标记按钮组组中的删除按钮
};

/**
 * @brief 选项触发按钮
 */
struct OptionTriggerButton{
    QString m_ID;
    Location m_location;
    BaseInfo m_iconInfo;
    BaseInfo m_buttonInfo;
};

/**
 * @brief 边框
 */
struct TheBorder{
    BaseInfo m_baseInfo;
};

struct TitleInfo{
    BaseInfo m_baseInfo;
};
/**
 * @brief 按钮组
 */
struct ButtonGroup{
    Location m_location;
    QString m_ID;
    QList<ButtonInfo> m_buttons;
};

/**
 * @brief 选项弹出框
 */
struct OptionPopUp
{
    QString m_ID;
    Location m_location;
    TheBorder m_border;
    TitleInfo m_titleInfo;
    ButtonGroup m_optionBtnGroup;
    ButtonGroup m_exitBtnGroup;
};

/*!
 * @brief 自定义下拉框按钮
 */
struct DropDownButtonData{
    DropDownButtonData(){}

    OptionTriggerButton m_triggerBtnOpen;//选项触发按钮打开状态
    OptionTriggerButton m_triggerBtnClose;//选项触发关闭状态
    OptionPopUp m_optionPopUp;//选项弹出框
    QString m_ID;

};

//////////////////自定义输入框//////////////////////
/**
 * @brief 各部分基础属性
 */
struct BassAttribute{
    QString m_ID;
    QString m_text;
    QString m_srcImage;
    Location m_location;
    IconInfo m_iconInfo;
};

/**
 * @brief按钮属性
 */
struct BUTTON{
    BUTTON():deleteBtn(false){}
   BassAttribute m_baseAtrribute;
   Location m_location;
   bool deleteBtn;
};

/**
 * @brief 触发输入框部分
 */
struct TriggerInputBox{
   BassAttribute m_baseAtrribute;
   Location m_location;
};

/**
 * @brief TITLE
 */
struct TITLE{
   BassAttribute m_baseAtrribute;
   bool m_tytle;
   Location m_location;
};
/**
 * @brief 虚拟键盘按键区
 */
struct THEKEYS{
   BassAttribute m_baseAtrribute;
   QList<BUTTON> m_keyBtns;
   Location m_location;
};
/**
 * @brief 尾部
 */
struct TAIL{
   BassAttribute m_baseAtrribute;
   QList<BUTTON> m_quitBtns;
   Location m_location;
};
/**
 * @brief 虚拟键盘部分
 */
struct VirtualKeyboard{
    BassAttribute m_baseAtrribute;
    TITLE m_title;
    THEKEYS m_keyGroup;
    TAIL m_tailGroup;
    Location m_location;
    QString m_ID;
};

struct ControlStyles{
    ControlStyles(){}
    ~ControlStyles(){}

    TriggerInputBox m_triggerInputBox;//触发编辑框
    VirtualKeyboard m_virtualKeyGroup;//虚拟键盘

};


/////////////////自定义双向滚动条////////////////////
/**
 * @brief 文字提示
 */
struct TheTooltip{
    QString m_ID;
    Location m_location;
    BaseInfo m_textBox;  /*!< 文本框 */
};

/**
 * @brief 滚动条
 */
struct DScrollBar{
    QString m_id;
    Location m_location;
    QList<BaseInfo> m_srcImgs;
};

struct BidirectionalSlider{
    BidirectionalSlider(){}
    ~BidirectionalSlider(){}

    QList<BaseInfo> m_rectangularsInfo;
    Location m_location;
    DScrollBar m_leftScrollBar;  /*!< 左滚动条 */
    DScrollBar m_rightScrollBar;     /*!< 右滚动条 */
    TheTooltip m_leftToolTip;        /*!< 左文字提示 */
    TheTooltip m_rightToolTip;        /*!< 左文字提示 */
    QString m_ID;
};

////////////////////自定义悬浮窗/////////////////////////////////
/**
 * @brief 开关按钮
 */
struct SwitchButton{
    BaseInfo m_openState;    /*!< 打开状态 */
    BaseInfo m_closeState;    /*!< 关闭状态 */
    QString m_id;
    Location m_location;
};
/**
 * @brief 主窗体部分：由开关按钮组成
 */
struct MainWidget{
    QList<SwitchButton> m_switchButtons;
    Location m_location;
    QString m_srcImg;
    QString m_id;
};

/**
 * @brief 悬浮按钮，控制界面的显示隐藏
 */
struct FloatButton{
    Location m_location;
    QList<BaseInfo> m_srcImgs;
};

struct FloatingWindow{
    FloatingWindow(){}
    ~FloatingWindow(){}

    MainWidget m_mainWidget;
    FloatButton m_floatButton;
    Location m_location;
    QString m_ID;
};


/////////////////////////////////////////////
/// \brief 自定义折叠信息控件
///

/**
 * @brief 折叠信息
 */
struct FoldingInfo{
    Location m_location;
    QString m_ID;
    QList<BaseInfo> m_information;
};

/**
 * @brief 展开信息
 */
struct UnFoldInfo{
    UnFoldInfo(): m_autoSetControl(true){}
    QString m_parentID;//当前展开信息所在的动态面板的id
    Location m_parentLocation;//当前展开信息所在动态面板的位置信息
    Location m_location;
    QString m_ID;
    QList<BaseInfo> m_information;
    bool m_autoSetControl;   /*!< 有些需要手动设置控件类型以及布局 */
};
struct Information{
    FoldingInfo m_foldingInfo;
    UnFoldInfo m_unFoldInfo;
    QString m_ID;
    Location m_location;
};
struct FoldingControls{
    FoldingControls():m_addScrollBar(true){}
    ~FoldingControls(){}

    QList<Information> m_informations;
    Location m_location;
    QString m_ID;
    bool m_addScrollBar;   /*!< 有些控件不需要添加滚动条 */
};

////////////////////////////////////////////////


struct customSwitchButton{
    customSwitchButton(){}

    QList<BaseInfo> m_textParagraphs;
    QList<BaseInfo> m_lineEdits;
    BaseInfo m_closeState;
    BaseInfo m_openState;
    QString m_ID;
    Location m_location;
};

////////////////单滑块//////////////////////////

struct InputBoxInfo{
    QString m_ID;
    Location m_location;
    QList<BaseInfo> m_inputBoxInfos;  /*!< 输入框 */
};
/**
 * @brief 自定义单滑块
 */
struct SingleSlidingBlockData{
    SingleSlidingBlockData(){}

    QList<BaseInfo> m_rectangularsInfo; /*!< 内部组成部分矩形信息 */
    DScrollBar m_leftScrollBar;  /*!< 左滑块 */
    TheTooltip m_letfToolTip;   /*!< 左文字提示 */
    InputBoxInfo m_inputBox;    /*!< 文字输入框 */
    Location m_location;
    QString m_ID;
};

/////////////////带有弹框的自定义标签控件//////////////////////、

/**
 * @brief 标签弹窗
 */
struct LabelPopupWindow
{
    QString m_ID;
    Location m_location;
    QList<BaseInfo> m_optionsInfo;   /*!< 弹窗选项 */
};

/**
 * @brief 带有弹框的自定义标签控件
 */

struct CustomLabelData{

    LabelPopupWindow m_labelPopupWindow;
    BaseInfo m_defaultInfo;  /*!< 标签默认状态信息 */
    BaseInfo m_mouseEnter;    /*!< 鼠标进入标签状态信息 */
};
#endif // HEAD_H
