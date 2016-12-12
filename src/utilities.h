#pragma once

#include "editshaderdialog.h"
#include "optionsdialog.h"
#include "messagebox.h"

#include <QApplication>
#include <QListWidget>
#include <QFile>
#include <qdebug.h>
#include <QListWidgetItem>
#include <QIcon>
#include <QSettings>

#ifdef Q_OS_WIN
    extern HICON qt_pixmapToWinHICON(const QPixmap &p);
#endif

// TODO: Verify ShatteredGlass (without spaces)
static QStringList gShaders = (QStringList() << "Cable" << "Decal" << "DecalModulate" << "Infected" << "LightmappedGeneric"
											 << "Modulate" << "MonitorScreen" << "Patch" << "Predator" << "Refract"
											 << "ShatteredGlass" << "Sprite" << "SpriteCard" << "UnlitGeneric" << "UnlitTwoTexture"
											 << "VertexLitGeneric" << "Water" << "WorldVertexTransition" << "Shattered Glass" << "Lightmapped_4WayBlend" );


#define Error(x) { qDebug() << QString(x); mLogger->addItem( new QListWidgetItem( QIcon(":/icons/error"), x )); mLogger->scrollToBottom(); }

#define Warning(x) { qDebug() << QString(x); mLogger->addItem( new QListWidgetItem( QIcon(":/icons/warning"), x )); mLogger->scrollToBottom(); }

#define Info(x) { qDebug() << QString(x); mLogger->addItem( new QListWidgetItem( QIcon(":/icons/info"), x )); mLogger->scrollToBottom(); }



#define X(x) { qDebug() << QString(x); mLogger->addItem( new QListWidgetItem( QIcon(":/icons/about"), "Error: " + QString(x) )); mLogger->scrollToBottom(); }

#define Y(x) { qDebug() << QString(x); }

#define Q(x) { qDebug() << "\n\n" << QString(x); MsgBox::warning( NULL, "Fatal error occurred!", QString(x) ); exit(1); }

#define Str(x) QString::number(x)


// TODO: Put setKey(), addDefaultShader() and something else I forgot in the classes
bool isWhitespace( const QString& input );

QString setKey( const QString& name, const QString& def, QSettings* settings );

bool setKey(const QString &name, bool def, QSettings *settings);

QString addTabs( int amount );

void removeSingleLineComment( QString& string );
