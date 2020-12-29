/*!
 *  @brief     文本解析/保存类
 *  @details   对不同的文本读取、保存做了一定的抽象，简化代码 @p
 *             @class RXmlParseMethod
 *                  解析不同的xml文件时，只需继承此类，并且根据需要实现对应的虚函数
 *
 *             @class RXmlFile
 *                  xml格式文件,采用装饰器模式对通用的信息进行处理
 *  @author    wey
 *  @version   1.0
 *  @date      2018.09.20
 *  @warning
 *  @copyright NanJing RenGu.
 */

#ifndef FILEUTILS_H
#define FILEUTILS_H

#include <QtXml/QDomElement>
#include <QFile>


class RFile : public QFile
{
    Q_OBJECT
public:
    RFile(const QString &fileName);

    virtual bool startParse(OpenMode openMode = QFile::ReadOnly);
    virtual bool startSave(OpenMode  openMode = QFile::WriteOnly | QFile::Truncate | QFile::Text);

protected:
    bool isAutoReleaseParseMethod;

};

/***********************************XML文件解析******************************************/

class RXmlParseMethod
{
public:
    RXmlParseMethod(){}
    virtual ~RXmlParseMethod(){}

    /*!
     * @brief 解析xml文件
     * @attention rootNode是已经打开的文件根节点，只需从此节点开始进一步解析子节点即可。
     * @param[in] rootNode 文档根节点
     * @return bool 是否解析成功
     */
    virtual bool  startParse(const QDomNode & /*rootNode*/){return true;}

    /*!
     * @brief 保存xml文件信息
     * @attention 用户需要在doc中创建根节点，并一次创建其它节点，文件的保存由基类处理
     * @param[in] doc 文档对象
     * @return true 保存成功
     */
    virtual bool  startSave(QDomDocument & /*doc*/){return true;}
};

class RXmlFile : public RFile
{
public:
    RXmlFile(const QString & fileName);
    ~RXmlFile();

    void disableAutoAddXmlSuffix();

    /*!
     * @brief 设置解析方式
     * @param[in] method 解析方式
     * @param[in] autoRelease 是否解析结束自动释放method的内存，默认为true
     */
    void setParseMethod(RXmlParseMethod * method,bool autoRelease = true){this->parseMethod = method;
                                                                          this->isAutoReleaseParseMethod = autoRelease;}

    virtual bool startParse(OpenMode  openMode = QFile::ReadOnly);
    virtual bool startSave(OpenMode  openMode = QFile::WriteOnly | QFile::Truncate | QFile::Text,bool autoAddProcessingNode = true);

protected:
    RXmlParseMethod * parseMethod;
    bool m_bAutoAddXmlSuffix;
};

/***********************************普通文件解析******************************************/

class RTextFile;

class RTextParseMethod
{
public:
    RTextParseMethod(){}
    virtual ~RTextParseMethod(){}

    /*!
     * @brief 解析普通文件
     * @param[in] file 文档对象
     * @return bool 是否解析成功
     */
    virtual bool  startParse(RTextFile * /*file*/){return true;}

    /*!
     * @brief 保存xml文件信息
     * @param[in] file 文档对象
     * @return true 保存成功
     */
    virtual bool  startSave(RTextFile * /*file*/){return true;}
};

class RTextFile : public RFile
{
public:
    RTextFile(const QString & fileName);
    ~RTextFile();

    void setParseMethod(RTextParseMethod * p,bool autoRelease = true){this->parseMethod = p;
                                                                     this->isAutoReleaseParseMethod = autoRelease;}

    virtual bool startParse(OpenMode  openMode = QFile::ReadOnly, bool isRecordLogflie = true);
    virtual bool startSave(OpenMode  openMode = QFile::WriteOnly | QFile::Truncate | QFile::Text, bool isRecordLogfile = true);

protected:
    RTextParseMethod * parseMethod;
};

#endif // FILEUTILS_H
