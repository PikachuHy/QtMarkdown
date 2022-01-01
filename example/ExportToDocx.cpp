//
// Created by PikachuHy on 2022/1/1.
//
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QXmlStreamWriter>

#include "JlCompress.h"
#include "parser/Document.h"
#include "parser/Parser.h"
#include "parser/Text.h"
#include "parser/Visitor.h"
#include "quazip.h"
#include "quazip_global.h"
#include "quazipfile.h"
#include "quazipfileinfo.h"

using namespace md;
using namespace md::parser;
class DocxRender : public MultipleVisitor<Header, Text, Paragraph
                                          /*
                                           * ItalicText, BoldText, ItalicBoldText, StrickoutText, Image, Link,
                                                               CodeBlock,
                                          InlineCode,
                                                               CheckboxList, CheckboxItem, UnorderedList,
                                          UnorderedListItem, OrderedList, OrderedListItem, LatexBlock, InlineLatex, Hr,
                                          QuoteBlock, Table, Lf
                                                               */
                                          > {
 public:
  DocxRender(DocPtr doc, QXmlStreamWriter& writer) : m_doc(doc), m_writer(writer) {}
  void writeElement(String name, std::function<void()> fn) {
    m_writer.writeStartElement(name);
    fn();
    m_writer.writeEndElement();
  }
  void visit(Header* node) override {
    writeElement("w:p", [this, node]() {
      writeElement("w:pPr",
                   [this]() { writeElement("w:pStyle", [this]() { m_writer.writeAttribute("w:val", "1"); }); });
      writeElement("w:r", [this, node]() {
        writeElement("w:rPr", [this]() {

        });
        writeElement("w:t", [this, node]() {
          for (auto child : node->children()) {
            child->accept(this);
          }
        });
      });
    });
  }
  void visit(Paragraph* node) override {
    m_writer.writeStartElement("w:p");
    /*
        m_writer.writeStartElement("w:pPr");
        m_writer.writeStartElement("w:pStyle");
        m_writer.writeAttribute("w:val", "Normal");
        m_writer.writeEndElement(); // w:pStyle
        m_writer.writeStartElement("w:jc");
        m_writer.writeAttribute("w:val", "left");
        m_writer.writeEndElement(); // w:jc
        m_writer.writeStartElement("w:rPr");
        m_writer.writeEndElement(); // w:rPr
        m_writer.writeEndElement(); // w:pPr
    */
    m_writer.writeStartElement("w:r");
    m_writer.writeStartElement("w:rPr");
    m_writer.writeEndElement();  // w:rPr
    m_writer.writeStartElement("w:t");
    for (auto child : node->children()) {
      child->accept(this);
    }
    m_writer.writeEndElement();  // w:t
    m_writer.writeEndElement();  // w:r

    m_writer.writeEndElement();  // w:p
  }
  void visit(Text* node) override {
    auto str = node->toString(m_doc);
    m_writer.writeCharacters(str);
  }

  DocPtr m_doc;
  QXmlStreamWriter& m_writer;
};
void makeSureDirExist(const QFile& file) {
  auto dirPath = QFileInfo(file).absolutePath();
  if (QDir(dirPath).exists()) {
    return;
  }
  qDebug() << "mkdir" << dirPath;
  bool ok = QDir().mkpath(dirPath);
  if (!ok) {
    qWarning() << "mkdir" << dirPath << "fail";
  }
}
void writeRel(String output) {
  QFile relFile(output + "/_rels/.rels");
  makeSureDirExist(relFile);
  if (!relFile.open(QIODevice::WriteOnly)) {
    qDebug() << "open file fail: " << relFile.fileName();
    return;
  }
  QXmlStreamWriter stream(&relFile);
  stream.setAutoFormatting(true);
  stream.writeStartDocument("1.0", true);
  stream.writeStartElement("Relationships");
  stream.writeAttribute("xmlns", "http://schemas.openxmlformats.org/package/2006/relationships");

  stream.writeStartElement("Relationship");
  stream.writeAttribute("Id", "rId3");
  stream.writeAttribute("Type",
                        "http://schemas.openxmlformats.org/officeDocument/2006/relationships/extended-properties");
  stream.writeAttribute("Target", "docProps/app.xml");
  stream.writeEndElement();

  stream.writeStartElement("Relationship");
  stream.writeAttribute("Id", "rId2");
  stream.writeAttribute("Type",
                        "http://schemas.openxmlformats.org/package/2006/relationships/metadata/core-properties");
  stream.writeAttribute("Target", "docProps/core.xml");
  stream.writeEndElement();

  stream.writeStartElement("Relationship");
  stream.writeAttribute("Id", "rId1");
  stream.writeAttribute("Type", "http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument");
  stream.writeAttribute("Target", "word/document.xml");
  stream.writeEndElement();

  stream.writeEndElement();
  stream.writeEndDocument();
}
void writeAppProps(String output) {
  QFile appFile(output + "/docProps/app.xml");
  makeSureDirExist(appFile);
  if (!appFile.open(QIODevice::WriteOnly)) {
    qDebug() << "open file fail: " << appFile.fileName();
    return;
  }
  QXmlStreamWriter stream(&appFile);
  stream.setAutoFormatting(true);
  stream.writeStartDocument("1.0", true);
  stream.writeStartElement("Properties");
  stream.writeAttribute("xmlns", "http://schemas.openxmlformats.org/officeDocument/2006/extended-properties");
  stream.writeAttribute("xmlns:vt", "http://schemas.openxmlformats.org/officeDocument/2006/docPropsVTypes");

  stream.writeTextElement("Template", "Normal.dotm");
  stream.writeTextElement("TotalTime", "0");
  stream.writeTextElement("Pages", "1");
  stream.writeTextElement("Words", "0");
  stream.writeTextElement("Application", "Microsoft Office Word");
  stream.writeTextElement("DocSecurity", "0");
  stream.writeTextElement("Lines", "0");
  stream.writeTextElement("Paragraphs", "0");
  stream.writeTextElement("ScaleCrop", "false");
  stream.writeTextElement("Company", "");
  stream.writeTextElement("LinksUpToDate", "false");
  stream.writeTextElement("CharactersWithSpaces", "");
  stream.writeTextElement("CompaSharedDocny", "false");
  stream.writeTextElement("HyperlinksChanged", "false");
  stream.writeTextElement("AppVersion", "16.0000");

  stream.writeEndElement();
  stream.writeEndDocument();
}
void writeCoreProps(String output) {
  QFile coreFile(output + "/docProps/core.xml");
  makeSureDirExist(coreFile);
  if (!coreFile.open(QIODevice::WriteOnly)) {
    qDebug() << "open file fail: " << coreFile.fileName();
    return;
  }
  QXmlStreamWriter stream(&coreFile);
  stream.setAutoFormatting(true);
  stream.writeStartDocument("1.0", true);

  stream.writeStartElement("cp:coreProperties");
  stream.writeAttribute("xmlns:cp", "http://schemas.openxmlformats.org/package/2006/metadata/core-properties");
  stream.writeAttribute("xmlns:dc", "http://purl.org/dc/elements/1.1/");
  stream.writeAttribute("xmlns:dcterms", "http://purl.org/dc/terms/");
  stream.writeAttribute("xmlns:dcmitype", "http://purl.org/dc/dcmitype/");
  stream.writeAttribute("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
  stream.writeEndElement();

  stream.writeTextElement("dc:title", "");
  stream.writeTextElement("dc:subject", "");
  stream.writeTextElement("dc:creator", "");
  stream.writeTextElement("cp:keywords", "");
  stream.writeTextElement("dc:description", "");
  stream.writeTextElement("cp:lastModifiedBy", "");
  stream.writeTextElement("cp:revision", "1");

  stream.writeStartElement("dcterms:created");
  stream.writeAttribute("xsi:type", "dcterms:W3CDTF");
  stream.writeCharacters("2022-01-01T05:32:00Z");
  stream.writeEndElement();

  stream.writeStartElement("dcterms:modified");
  stream.writeAttribute("xsi:type", "dcterms:W3CDTF");
  stream.writeCharacters("2022-01-01T05:32:00Z");
  stream.writeEndElement();

  stream.writeEndDocument();
}
void writeProps(String output) {
  writeAppProps(output);
  writeCoreProps(output);
}
void writeWebSettings(String output) {
  QFile coreFile(output + "/word/webSettings.xml");
  makeSureDirExist(coreFile);
  if (!coreFile.open(QIODevice::WriteOnly)) {
    qDebug() << "open file fail: " << coreFile.fileName();
    return;
  }
  QXmlStreamWriter stream(&coreFile);
  stream.setAutoFormatting(true);
  stream.writeStartDocument("1.0", true);

  stream.writeStartElement("w:webSettings");
  stream.writeAttribute("xmlns:mc", "http://schemas.openxmlformats.org/markup-compatibility/2006");
  stream.writeAttribute("xmlns:r", "http://schemas.openxmlformats.org/officeDocument/2006/relationships");
  stream.writeAttribute("xmlns:w", "http://schemas.openxmlformats.org/wordprocessingml/2006/main");
  stream.writeAttribute("xmlns:w14", "http://schemas.microsoft.com/office/word/2010/wordml");
  stream.writeAttribute("xmlns:w15", "http://schemas.microsoft.com/office/word/2012/wordml");
  stream.writeAttribute("xmlns:w16cex", "http://schemas.microsoft.com/office/word/2018/wordml/cex");
  stream.writeAttribute("xmlns:w16cid", "http://schemas.microsoft.com/office/word/2016/wordml/cid");
  stream.writeAttribute("xmlns:w16", "http://schemas.microsoft.com/office/word/2018/wordml");
  stream.writeAttribute("xmlns:w16se", "http://schemas.microsoft.com/office/word/2015/wordml/symex");
  stream.writeAttribute("mc:Ignorable", "w14 w15 w16se w16cid w16 w16cex");

  stream.writeStartElement("w:optimizeForBrowser");
  stream.writeEndElement();
  stream.writeStartElement("w:allowPNG");
  stream.writeEndElement();

  stream.writeEndElement();

  stream.writeEndDocument();
}
void writeWord(String mdPath, String output) {
  QFile mdFile(mdPath);
  if (!mdFile.open(QIODevice::ReadOnly)) {
    qDebug() << "open file fail:" << mdPath;
    return;
  }
  String mdText = mdFile.readAll();
  QFile coreFile(output + "/word/document.xml");
  makeSureDirExist(coreFile);
  if (!coreFile.open(QIODevice::WriteOnly)) {
    qDebug() << "open file fail: " << coreFile.fileName();
    return;
  }
  QXmlStreamWriter stream(&coreFile);
  stream.setAutoFormatting(true);
  stream.writeStartDocument("1.0", true);

  stream.writeStartElement("w:document");
  stream.writeAttribute("xmlns:wpc", "http://schemas.microsoft.com/office/word/2010/wordprocessingCanvas");
  stream.writeAttribute("xmlns:cx", "http://schemas.microsoft.com/office/drawing/2014/chartex");
  stream.writeAttribute("xmlns:cx1", "http://schemas.microsoft.com/office/drawing/2015/9/8/chartex");
  stream.writeAttribute("xmlns:cx2", "http://schemas.microsoft.com/office/drawing/2015/10/21/chartex");
  stream.writeAttribute("xmlns:cx3", "http://schemas.microsoft.com/office/drawing/2016/5/9/chartex");
  stream.writeAttribute("xmlns:cx4", "http://schemas.microsoft.com/office/drawing/2016/5/10/chartex");
  stream.writeAttribute("xmlns:cx5", "http://schemas.microsoft.com/office/drawing/2016/5/11/chartex");
  stream.writeAttribute("xmlns:cx6", "http://schemas.microsoft.com/office/drawing/2016/5/12/chartex");
  stream.writeAttribute("xmlns:cx7", "http://schemas.microsoft.com/office/drawing/2016/5/13/chartex");
  stream.writeAttribute("xmlns:cx8", "http://schemas.microsoft.com/office/drawing/2016/5/14/chartex");
  stream.writeAttribute("xmlns:mc", "http://schemas.openxmlformats.org/markup-compatibility/2006");
  stream.writeAttribute("xmlns:aink", "http://schemas.microsoft.com/office/drawing/2016/ink");
  stream.writeAttribute("xmlns:am3d", "http://schemas.microsoft.com/office/drawing/2017/model3d");
  stream.writeAttribute("xmlns:o", "urn:schemas-microsoft-com:office:office");
  stream.writeAttribute("xmlns:r", "http://schemas.openxmlformats.org/officeDocument/2006/relationships");
  stream.writeAttribute("xmlns:m", "http://schemas.openxmlformats.org/officeDocument/2006/math");
  stream.writeAttribute("xmlns:v", "urn:schemas-microsoft-com:vml");
  stream.writeAttribute("xmlns:wp14", "http://schemas.microsoft.com/office/word/2010/wordprocessingDrawing");
  stream.writeAttribute("xmlns:wp", "http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing");
  stream.writeAttribute("xmlns:w10", "urn:schemas-microsoft-com:office:word");
  stream.writeAttribute("xmlns:w", "http://schemas.openxmlformats.org/wordprocessingml/2006/main");
  stream.writeAttribute("xmlns:w14", "http://schemas.microsoft.com/office/word/2010/wordml");
  stream.writeAttribute("xmlns:w15", "http://schemas.microsoft.com/office/word/2012/wordml");
  stream.writeAttribute("xmlns:w16cex", "http://schemas.microsoft.com/office/word/2018/wordml/cex");
  stream.writeAttribute("xmlns:w16cid", "http://schemas.microsoft.com/office/word/2016/wordml/cid");
  stream.writeAttribute("xmlns:w16", "http://schemas.microsoft.com/office/word/2018/wordml");
  stream.writeAttribute("xmlns:w16se", "http://schemas.microsoft.com/office/word/2015/wordml/symex");
  stream.writeAttribute("xmlns:wpg", "http://schemas.microsoft.com/office/word/2010/wordprocessingGroup");
  stream.writeAttribute("xmlns:wpi", "http://schemas.microsoft.com/office/word/2010/wordprocessingInk");
  stream.writeAttribute("xmlns:wne", "http://schemas.microsoft.com/office/word/2006/wordml");
  stream.writeAttribute("xmlns:wps", "http://schemas.microsoft.com/office/word/2010/wordprocessingShape");
  stream.writeAttribute("mc:Ignorable", "w14 w15 w16se w16cid w16 w16cex wp14");

  stream.writeStartElement("w:body");
  /*
    stream.writeStartElement("w:p");
    stream.writeAttribute("w14:paraId", "7AC88C37");
    stream.writeAttribute("w14:textId", "77777777");
    stream.writeAttribute("w:rsidR", "00371AA9");
    stream.writeAttribute("w:rsidRDefault", "000B544C");

    stream.writeStartElement("w:sectPr");
    stream.writeAttribute("w:rsidR", "00371AA9");

    stream.writeStartElement("w:pgSz");
    stream.writeAttribute("w:w", "11906");
    stream.writeAttribute("w:h", "16838");
    stream.writeEndElement(); // w:pgSz

    stream.writeStartElement("w:pgMar");
    stream.writeAttribute("w:top", "1440");
    stream.writeAttribute("w:right", "1800");
    stream.writeAttribute("w:bottom", "1440");
    stream.writeAttribute("w:left", "1800");
    stream.writeAttribute("w:header", "851");
    stream.writeAttribute("w:footer", "992");
    stream.writeAttribute("w:gutter", "0");
    stream.writeEndElement(); // w:pgMar

    stream.writeStartElement("w:cols");
    stream.writeAttribute("w:space", "425");
    stream.writeEndElement(); // w:cols

    stream.writeStartElement("w:docGrid");
    stream.writeAttribute("w:type", "lines");
    stream.writeAttribute("w:linePitch", "312");
    stream.writeEndElement();

    stream.writeEndElement(); // w:sectPr
  */
  Document doc(mdText);
  DocxRender render(&doc, stream);
  doc.accept(&render);

  stream.writeEndElement();  // w:body

  stream.writeEndElement();  // w:document

  stream.writeEndDocument();
}
void writeContentTypes(String output) {
  QFile coreFile(output + "/[Content_Types].xml");
  makeSureDirExist(coreFile);
  if (!coreFile.open(QIODevice::WriteOnly)) {
    qDebug() << "open file fail: " << coreFile.fileName();
    return;
  }
  QXmlStreamWriter stream(&coreFile);
  stream.setAutoFormatting(true);
  stream.writeStartDocument("1.0", true);

  stream.writeStartElement("Types");
  stream.writeAttribute("xmlns", "http://schemas.openxmlformats.org/package/2006/content-types");

  stream.writeStartElement("Default");
  stream.writeAttribute("Extension", "rels");
  stream.writeAttribute("ContentType", "application/vnd.openxmlformats-package.relationships+xml");
  stream.writeEndElement();

  stream.writeStartElement("Default");
  stream.writeAttribute("Extension", "xml");
  stream.writeAttribute("ContentType", "application/xml");
  stream.writeEndElement();

  stream.writeStartElement("Override");
  stream.writeAttribute("PartName", "/word/document.xml");
  stream.writeAttribute("ContentType",
                        "application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml");
  stream.writeEndElement();

  stream.writeStartElement("Override");
  stream.writeAttribute("PartName", "/word/styles.xml");
  stream.writeAttribute("ContentType", "application/vnd.openxmlformats-officedocument.wordprocessingml.styles+xml");
  stream.writeEndElement();

  stream.writeStartElement("Override");
  stream.writeAttribute("PartName", "/word/settings.xml");
  stream.writeAttribute("ContentType", "application/vnd.openxmlformats-officedocument.wordprocessingml.settings+xml");
  stream.writeEndElement();

  stream.writeStartElement("Override");
  stream.writeAttribute("PartName", "/word/webSettings.xml");
  stream.writeAttribute("ContentType",
                        "application/vnd.openxmlformats-officedocument.wordprocessingml.webSettings+xml");
  stream.writeEndElement();

  stream.writeStartElement("Override");
  stream.writeAttribute("PartName", "/word/fontTable.xml");
  stream.writeAttribute("ContentType", "application/vnd.openxmlformats-officedocument.wordprocessingml.fontTable+xml");
  stream.writeEndElement();

  stream.writeStartElement("Override");
  stream.writeAttribute("PartName", "/word/theme/theme1.xml");
  stream.writeAttribute("ContentType", "application/vnd.openxmlformats-officedocument.theme+xml");
  stream.writeEndElement();

  stream.writeStartElement("Override");
  stream.writeAttribute("PartName", "/docProps/core.xml");
  stream.writeAttribute("ContentType", "application/vnd.openxmlformats-package.core-properties+xml");
  stream.writeEndElement();

  stream.writeStartElement("Override");
  stream.writeAttribute("PartName", "/docProps/app.xml");
  stream.writeAttribute("ContentType", "application/vnd.openxmlformats-officedocument.extended-properties+xml");
  stream.writeEndElement();

  stream.writeEndElement();

  stream.writeEndDocument();
}
void mdToDocx(String mdPath, String output) {
  writeContentTypes(output);
  writeRel(output);
  writeProps(output);
  writeWord(mdPath, output);
}
static bool copyData(QIODevice& inFile, QIODevice& outFile) {
  while (!inFile.atEnd()) {
    char buf[4096];
    qint64 readLen = inFile.read(buf, 4096);
    if (readLen <= 0) return false;
    if (outFile.write(buf, readLen) != readLen) return false;
  }
  return true;
}
bool writeFile(String output, String filePath, QuaZip& quaZip) {
  QFile docFile(output + "/" + filePath);
  if (!docFile.open(QIODevice::ReadOnly)) {
    qDebug() << "open file fail: " << docFile.fileName();
    return false;
  }
  QuaZipNewInfo info(filePath, output + "/" + filePath);
  QuaZipFile quaZipFile(&quaZip);
  if (!quaZipFile.open(QIODevice::WriteOnly, info)) {
    qDebug() << "open qua zip file fail: " << quaZipFile.getFileName();
    return false;
  }
  bool ok = copyData(docFile, quaZipFile);
  if (!ok) {
    qDebug() << "copy data error: " << filePath;
    return false;
  }
  docFile.close();
  quaZipFile.close();
  return true;
}
void mkDocxFile(String output) {
  QuaZip quaZip(output + "/../my.docx");
  if (!quaZip.open(QuaZip::mdCreate)) {
    qDebug() << "open file fail:" << quaZip.getCurrentFileName();
    return;
  }
  QStringList pathList;
  pathList << "[Content_Types].xml";
  pathList << "_rels/.rels";
  pathList << "word/_rels/document.xml.rels";
  pathList << "word/document.xml";
  pathList << "word/theme/theme1.xml";
  pathList << "word/settings.xml";
  pathList << "docProps/core.xml";
  pathList << "word/fontTable.xml";
  pathList << "word/webSettings.xml";
  pathList << "word/styles.xml";
  pathList << "docProps/app.xml";

  for (const auto& path : pathList) {
    bool ok = writeFile(output, path, quaZip);
    qDebug() << "writing " << path;
    if (!ok) {
      qDebug() << "write fail:" << path;
      break;
    }
  }
  quaZip.close();
}
int main() {
  String output = "output";
  //  mdToDocx("", output);
  String my_path = "my";
  writeWord("test.md", my_path);
  mkDocxFile(my_path);
  return 0;
}