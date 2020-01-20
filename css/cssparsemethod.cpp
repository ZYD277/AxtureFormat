#include "cssparsemethod.h"

#include <QDebug>
#include <QTextStream>
#include <QRegExp>
#include <QList>

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
    QTextStream stream(file);

    m_cssMap.clear();

    CssSegment segment;

    while(!stream.atEnd()){
        QString oneLine = stream.readLine().trimmed();
        if(oneLine.size() > 0){
            if(oneLine.contains(QRegExp("\\{"))){
                QString selectorName = oneLine.left(oneLine.indexOf(QRegExp("\\{"))).trimmed();

                segment.type = Element;
                if(selectorName.startsWith(".")){
                    selectorName = selectorName.right(selectorName.size() - selectorName.indexOf(QRegExp("\\.")) - 1);
                    segment.type = Clazz;
                }else if(selectorName.startsWith("#")){
                    selectorName = selectorName.right(selectorName.size() - selectorName.indexOf("#") - 1);
                    segment.type = Id;
                }

                segment.selectorName = selectorName;
                segment.rules.clear();
            }else if(oneLine.contains(QRegExp("\\}"))){
                m_cssMap.insert(segment.selectorName,segment);
            }else{
                if(oneLine.contains(";")){
                    oneLine = oneLine.left(oneLine.indexOf(";")).trimmed();
                }

                QStringList tlist = oneLine.split(QRegExp(":"));
                if(tlist.size() == 2){
                    CssRule rule;
                    rule.name = tlist.at(0).trimmed();
                    rule.value = tlist.at(1).trimmed();
                    segment.rules.append(rule);
                }
            }
        }
    }

//    for(int i = 0; i < cssList.size(); i++){
//        CssSegment cs = cssList.at(i);
//        qDebug()<<cs.selectorName<<cs.type;
//        foreach(const CssRule & rule,cs.rules){
//            qDebug()<<rule.name<<rule.value;
//        }

//        qDebug()<<"=====================";
//    }

    return true;
}

} //namespace CSS
