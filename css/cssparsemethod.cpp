#include "cssparsemethod.h"

#include <QDebug>
#include <QTextStream>
#include <QRegExp>
#include <QList>
#include <QTextCodec>

namespace CSS{

CssParseMethod::CssParseMethod()
{

}

/*!
 * @brief 解析css文件
 * @attention 本解析直接使用字符串按行解析、提取，适用范围在
 * @param[in] file css文本文件
 * @return true:解析成功
 */
bool CssParseMethod::startParse(RTextFile *file)
{
    m_cssMap.clear();
    m_errorMsg.parse = false;
    m_errorMsg.getErrorMsg.clear();

    CssSegment segment;

    QString cssDataStr = file->readAll();

    QString key;
    QString value;
    int t_leftBra = 0;            /*!< 左大括号当前位置*/
    int t_rightBra = -1;          /*!< 右大括号当前位置*/
    int t_leftBraLab = 0;         /*!< 左大括号标签*/
    int t_rightBraLab = 0;        /*!< 右大括号标签*/
    int t_colon = 0;              /*!< 冒号当前位置*/
    int t_semico = 0;             /*!< 分号当前位置*/
    int t_colonLab = 0;           /*!< 冒号标签*/
    int t_semicoLab = 0;          /*!< 分号标签*/
    int t_sizeLab = 0;            /*!< 当前数据执行位置标签*/
    int t_startDataLab = 0;

    for(int i = 0; i < cssDataStr.size(); i++)
    {
        if(cssDataStr.at(i) == "{")
        {
            t_leftBraLab++;
            t_leftBra = i;
            int startData;
            int endData;
            QString chooser = specialDis(cssDataStr,t_rightBra,t_leftBra);

            if(chooser.contains("/*"))
            {
                 if(chooser.contains("*/"))
                 {
                     for(int i = 0;i < chooser.size(); i++)
                     {
                         if(chooser.at(i) == "/")
                             t_startDataLab++;
                         if(t_startDataLab == 1)
                             startData = i;
                         else if(t_startDataLab == 2)
                         {
                             endData = i;
                             break;
                         }
                     }
                 }
                 else
                     break;
                 chooser = chooser.mid(endData+1,chooser.size()-endData);
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
                    selectorName = chooser.remove("#");
                    segment.type = Clazz;
                }
                else
                {
                    selectorName = chooser.remove("#");
                    segment.type = Id;
                }
            }
            else
            {
                selectorName = chooser;
            }
            segment.selectorName = selectorName;
            segment.rules.clear();
        }
        else if(cssDataStr.at(i) == ":")
        {
            t_colon = i;

            /*!< 第一个选择器没有*/
            if(t_leftBra == 0)
            {
                m_errorMsg.getErrorMsg = "error:"+QStringLiteral("第一个选择器没有'{'");
                break;
            }

            if(t_colonLab == 0 && t_semicoLab == 0)
            {
                key = cssDataStr.mid(t_leftBra+1,t_colon-t_leftBra-1);
            }
            else if(t_colonLab == t_semicoLab)
            {
                key = cssDataStr.mid(t_semico+1,t_colon-t_semico-1);
            }

            /*!< 当前选择器的Key含有问题*/
            if(!dataKey(key))
            {
                m_errorMsg.getErrorMsg = "error: "+segment.selectorName+QStringLiteral("选择器的Key含有问题");
                break;
            }
            else
            {
                if(key.contains("/*"))
                {
                    int startData;
                    int endData;
                    if(key.contains("*/"))
                    {
                        t_startDataLab = 0;
                        for(int i = 0;i < key.size(); i++)
                        {
                            if(key.at(i) == "/")
                                t_startDataLab++;
                            if(t_startDataLab == 1)
                                startData = i;
                            else if(t_startDataLab == 2)
                            {
                                endData = i;
                                break;
                            }
                        }
                    }
                    else
                        break;
                    key = key.mid(endData+1,key.size()-startData);
                }
                key = key.remove("\r\n").remove(" ");
            }

            /*!< 当前选择器没有‘}’，下一个选择器没有‘{’*/
            if(key.startsWith("#")&&!key.contains("{"))
            {
                m_errorMsg.getErrorMsg = "error: "+segment.selectorName+QStringLiteral("选择器没有‘}’，下一个选择器没有‘{’");
                break;
            }
            t_colonLab++;
        }
        else if(cssDataStr.at(i) == ";")
        {
            t_semicoLab++;
            int startData;
            int endData;
            if(t_semicoLab == t_colonLab)
            {
                t_semico = i;
                value = cssDataStr.mid(t_colon+1,t_semico-t_colon-1);
                value = value.remove("\r\n");
                for(int i = 0 ; i < value.size() ; i++)
                {
                    if(value.at(i) == " ");
                    else
                    {
                        startData = i;
                        break;
                    }
                }
                for(int i = 0 ; i < value.size() ; i++)
                {
                    if(value.at(i) == " ");
                    else
                    {
                        endData = i;
                    }
                }
                value = value.mid(startData,endData - startData+1);

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
        else if(cssDataStr.at(i) == "}")
        {
            t_rightBraLab++;
            if(t_rightBraLab == t_leftBraLab)
            {
                if(t_semicoLab == t_colonLab)
                {
                    t_rightBra = i;
                    m_cssMap.insert(segment.selectorName,segment);
                    QString dataExc = specialDis(cssDataStr,t_semico,t_rightBra);

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
    charaData = character.mid(startPosition+1,endPosition-startPosition-1);
    charaData = charaData.remove("\r\n").remove(" ");
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

} //namespace CSS
