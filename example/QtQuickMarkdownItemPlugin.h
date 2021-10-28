//
// Created by pikachu on 5/22/2021.
//

#ifndef QTMARKDOWN_QTQUICKMARKDOWNITEMPLUGIN_H
#define QTMARKDOWN_QTQUICKMARKDOWNITEMPLUGIN_H

#include <QQmlEngineExtensionPlugin>
class QtQuickMarkdownItemPlugin : public QQmlEngineExtensionPlugin{
Q_OBJECT
    Q_PLUGIN_METADATA(IID QQmlEngineExtensionInterface_iid)
public:
    void initializeEngine(QQmlEngine *engine, const char *uri) override;
};


#endif //QTMARKDOWN_QTQUICKMARKDOWNITEMPLUGIN_H
