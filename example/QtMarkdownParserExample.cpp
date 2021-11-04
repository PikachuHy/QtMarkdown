#include "Parser.h"
#include "Document.h"
#include <QDebug>
#include <QFile>
using namespace md::parser;
int main() {
    QFile mdFile(":/test.md");
    if (!mdFile.exists()) {
        qDebug() << "file not exist:" << mdFile.fileName();
        return 0;
    }
    mdFile.open(QIODevice::ReadOnly);
    auto mdText = mdFile.readAll();
    mdFile.close();
    qDebug().noquote().nospace() << mdText;
    auto doc = Parser().parse(mdText);
    auto html = doc->toHtml();
    qDebug().noquote() << html;
    QFile htmlFile("index.html");
    htmlFile.open(QIODevice::WriteOnly);
    html = R"(<!DOCTYPE html>
<html>
<head>
<meta charset="utf-8">
<title>Markdown</title>
<link rel="stylesheet" href="github-markdown.css">
</head><body>
<article class="markdown-body">)"
           +
           html
           +
           R"(</article></body></html>)";
    htmlFile.write(html.toUtf8());
    htmlFile.close();
    return 0;
}