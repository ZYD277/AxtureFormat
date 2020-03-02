#include "cssparsemethod.h"

#include <QDebug>
#include <QTextStream>
#include <QRegExp>
#include <QList>
#include <QTextCodec>
#include <QStack>

namespace CSS{

CssParseMethod::CssParseMethod()
{
    m_interateStyleMap.insert("mouseover",DMouseOver);
    m_interateStyleMap.insert("mousedown",DMouseDown);
    m_interateStyleMap.insert("selected",DSelected);
    m_interateStyleMap.insert("disabled",DDisabled);
}

/*!
 * @brief 解析css文件
 * @attention 本解析直接使用字符串按行解析、提取
 * @param[in] file css文本文件
 * @return true:解析成功
 */
bool CssParseMethod::startParse(RTextFile *file)
{
    m_cssMap.clear();
    m_errorMsg.parse = false;
    m_errorMsg.getErrorMsg.clear();

    if(!parseFile(file))
        return false;

    if(0){
        auto iter = m_cssMap.begin();
        while(iter != m_cssMap.end()){
            qDebug()<<iter.key();

            const CssSegment & seg = iter.value();
            for(int i= 0;i<seg.rules.size();i++){
                qDebug()<<seg.rules.at(i).name<<":"<<seg.rules.at(i).value;
            }
            qDebug()<<"===============================";

            ++iter;
        }
    }

    return true;
}

/*!
 * @brief 提取文件中所有css样式
 * @attention 按照规则执行初步提取，可对选择器类型、样式规则进行判定，封装成独立的单元。
 * @param[in] file css文件
 * @return true:提取成功；false:提取中出错
 */
bool CssParseMethod::parseFile(RTextFile *file)
{
    CssSegment segment;

    QString cssDataStr = file->readAll();

    QString key;
    int t_leftBraPos = 0;         /*!< 左大括号当前位置*/
    int t_rightBraPos = -1;       /*!< 右大括号当前位置*/
    int t_leftBraLab = 0;         /*!< 左大括号标签*/
    int t_rightBraLab = 0;        /*!< 右大括号标签*/
    int t_colonPos = 0;           /*!< 冒号当前位置*/
    int t_semicoPos = 0;          /*!< 分号当前位置*/
    int t_colonLab = 0;           /*!< 冒号标签*/
    int t_semicoLab = 0;          /*!< 分号标签*/
    int t_sizeLab = 0;            /*!< 当前数据执行位置标签*/

    for(int i = 0; i < cssDataStr.size(); i++)
    {
        const QChar curChar = cssDataStr.at(i);
        if(curChar == "{")
        {
            t_leftBraLab++;
            t_leftBraPos = i;
            QString chooser = specialDis(cssDataStr,t_rightBraPos,t_leftBraPos);

            if(!removeComments(chooser)){
                break;
            }

            /*!< 选择器*/
            QString selectorName;
            segment.type = Element;

            if(chooser.startsWith("."))
            {
                selectorName = chooser.remove(".");
                segment.type = Clazz;
            }
            else if(chooser.startsWith("#"))
            {
                if(chooser.contains("."))
                {
                    //#xx.mouseOver 交互属性格式
                    static QRegExp exp("\\.(\\w+)$");
                    int matchPos = 0;
                    int indexPos = exp.indexIn(chooser,matchPos);        //提取交互式类型
                    if(indexPos >= 0){
                        segment.type = DynamicType;
                        segment.dtype = getDynamicType(exp.cap(1));
                    }else{
                        segment.type = Clazz;
                    }
                }
                else
                {
                    //FIXED 修复在选择器中包含:分号，示例#u409_input:disabled
                    if(chooser.contains(":") && t_colonLab > 0){
                        --t_colonLab;

                        static QRegExp exp(":(\\w+)$");
                        int matchPos = 0;
                        int indexPos = exp.indexIn(chooser,matchPos);        //提取交互式类型disabled
                        if(indexPos >= 0){
                            segment.type = DynamicType;
                            segment.dtype = getDynamicType(exp.cap(1));
                        }
                    }else{
                        segment.type = Id;
                    }
                }
                selectorName = chooser.remove("#");
            }
            else
            {
                selectorName = chooser;
            }

            segment.selectorName = selectorName;
            segment.rules.clear();
        }
        else if(curChar == ":")
        {
            t_colonPos = i;

            /*!< 第一个选择器没有*/
            if(t_leftBraPos == 0){
                m_errorMsg.getErrorMsg = "error:"+QStringLiteral("第一个选择器没有'{'");
                break;
            }

            if(t_colonLab == 0 && t_semicoLab == 0){
                key = cssDataStr.mid(t_leftBraPos+1,t_colonPos - t_leftBraPos-1);
            }else if(t_colonLab == t_semicoLab){
                key = cssDataStr.mid(t_semicoPos + 1,t_colonPos - t_semicoPos -1);
            }

            /*!< 当前选择器的Key含有问题*/
            if(!dataKey(key)){
                m_errorMsg.getErrorMsg = "error: "+segment.selectorName+QStringLiteral("选择器的Key含有问题");
                break;
            }

            if(!removeComments(key)){
                break;
            }
            key = key.trimmed();

            /*!< 当前选择器没有‘}’，下一个选择器没有‘{’*/
            if(key.startsWith("#")&&!key.contains("{")){
                m_errorMsg.getErrorMsg = "error: "+segment.selectorName+QStringLiteral("选择器没有‘}’，下一个选择器没有‘{’");
                break;
            }
            t_colonLab++;
        }
        else if(curChar == ";")
        {
            t_semicoLab++;
            if(t_semicoLab == t_colonLab)
            {
                t_semicoPos = i;

                QString value = cssDataStr.mid(t_colonPos + 1,t_semicoPos - t_colonPos - 1).trimmed();
                if(!removeComments(value)){
                    break;
                }

                /*!< 属性信息*/
                CssRule rule;
                rule.name = key;
                rule.value = value;
                segment.rules.append(rule);
            }
            /*!< 当前选择器中确少‘:’*/
            else if(t_semicoLab > t_colonLab)
            {
                m_errorMsg.getErrorMsg = "error: "+segment.selectorName+QStringLiteral("选择器中确少‘:’");
                break;
            }
            /*!< 当前选择器中确少‘;’*/
            else if(t_semicoLab < t_colonLab)
            {
                m_errorMsg.getErrorMsg = "error: "+segment.selectorName+QStringLiteral("选择器中确少‘;’");
                break;
            }
        }
        else if(curChar == "}")
        {
            t_rightBraLab++;
            if(t_rightBraLab == t_leftBraLab)
            {
                if(t_semicoLab == t_colonLab)
                {
                    t_rightBraPos = i;
                    m_cssMap.insert(segment.selectorName,segment);
                    QString dataExc = specialDis(cssDataStr,t_semicoPos,t_rightBraPos);

                    /*!< 提示：当前选择器的‘}’之前数据异常*/
                    if(!dataExc.isEmpty()&&t_semicoLab!=0)
                    {
                        if(!m_errorMsg.getErrorMsg.isEmpty())
                            m_errorMsg.getErrorMsg = m_errorMsg.getErrorMsg+"///" + "warning: " +segment.selectorName+QStringLiteral("选择器的‘}’之前数据异常");
                        else
                            m_errorMsg.getErrorMsg = "warning: " +segment.selectorName+QStringLiteral("选择器的‘}’之前数据异常");
                    }
                    t_colonLab = 0;
                    t_semicoLab = 0;
                }
                /*!< 当前选择器缺少最后一个‘;’*/
                else
                {
                    m_errorMsg.getErrorMsg = "error: "+segment.selectorName+QStringLiteral("选择器缺少最后一个‘;’");
                    break;
                }
            }
            /*!< 当前选择器缺少‘}’*/
            else if(t_rightBraLab < t_leftBraLab)
            {
                QStringList listSele = segment.selectorName.split("{");
                m_errorMsg.getErrorMsg = "error: "+listSele[0]+QStringLiteral("选择器缺少‘}’");
                break;
            }
            /*!< 下一个选择器没有‘{’*/
            else if(t_rightBraLab > t_leftBraLab)
            {
                m_errorMsg.getErrorMsg = "error: "+segment.selectorName+QStringLiteral("的下一个选择器缺少‘{’");
                break;
            }
        }
        t_sizeLab = i;
    }

    /*!< 数据出现异常情况未解析完成*/
    if(t_sizeLab < cssDataStr.size() - 1)
        m_errorMsg.parse = false;
    else
        m_errorMsg.parse = true;

    return m_errorMsg.parse;
}

/*!
 * @brief 获取动态属性类型
 * @param[in] dyname 属性名
 * @return 对应的属性类型
 */
DynamicPropType CssParseMethod::getDynamicType(QString dyname)
{
    if(m_interateStyleMap.contains(dyname.toLower())){
        return m_interateStyleMap.value(dyname.toLower());
    }
    return DInvalid;
}

/**
 * @brief 处理数据中的换行空格符号
 * @param character 所需要处理的数据
 * @param startPosition 数据的起始位置
 * @param endPosition 数据的终止位置
 * @return 处理后的数据
 */
QString CssParseMethod::specialDis(const QString &character, int startPosition, int endPosition)
{
    QString charaData;
    charaData = character.mid(startPosition+1,endPosition-startPosition-1).trimmed();
    return charaData;
}

/**
 * @brief 针对map中的key的特殊处理
 * @param keyData 所处理的数据
 * @return
 */
bool CssParseMethod::dataKey(const QString &keyData)
{
    if(keyData.contains("/*")&&keyData.contains("*/"))
        return true;
    if(keyData.contains("\r\n"))
    {
        QStringList keyDataList = keyData.split("\r\n");
        int keyDataLab = 0;

        /*!< 异常删除了key出现不正常的key的情况*/
        if(!keyData.contains(":")&&!keyData.contains(";")&&!keyData.contains("#")&&!keyData.contains("{"))
        {
            for(int i = 0 ; i < keyDataList.size() ; i++)
            {
                QString keyDataStr = keyDataList[i];
                keyDataStr.remove(" ");

                if(keyDataStr != ""&&keyDataStr != "\t")
                {
                    if(keyDataList.size()-i > 1)
                    {
                        keyDataLab++;
                        break;
                    }
                }
            }
        }

        if(keyDataLab > 0)
            return false;
    }
    return true;
}

/*!
 * @brief 移除多行注释
 * @attention CSS中只支持多行注释，不支持单行注释;
 *            一行语句中可能包含多个注释，需全部移除
 * @param[in] originData 待处理数据
 * @return true:处理成功;false:处理失败
 */
bool CssParseMethod::removeComments(QString & originData)
{
    QStack<MultiComment> commentQueue;
    MultiComment comment;
    bool t_bCommentStart = false;
    bool t_bError = false;

    for(int i = 0;i < originData.size() && !t_bError;++i)
    {
        if(originData.at(i) == '/'){
            if(i < originData.size() - 1){
                if(originData.at(i+1) == '*'){
                    if(!t_bCommentStart){
                        t_bCommentStart = true;
                        comment.startPos = i;
                        ++i;
                    }
                }else if(originData.at(i+1) == '/'){
                    if(!t_bCommentStart){
                        t_bError = true;
                    }
                }
            }
        }else if(originData.at(i) == '*'){
            if(i < originData.size() - 1){
                if(originData.at(i+1) == '/'){
                    if(t_bCommentStart && (i+1) - comment.startPos > 2){ //避免出现 /*/ 情形
                        t_bCommentStart = false;
                        comment.endPos = i+1;
                        commentQueue.push(comment);
                        ++i;
                    }
                }
            }
        }
    }

    //若只有开头，没有结尾也任务错误
    if(t_bError || t_bCommentStart)
        return false;

    while(commentQueue.size() > 0){
        MultiComment & com = commentQueue.pop();
        originData = originData.remove(com.startPos,com.endPos - com.startPos + 1);
    }

    originData = originData.trimmed();

    return true;
}

} //namespace CSS
