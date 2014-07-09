/********************************************************************
**  Nulloy Music Player, http://nulloy.com
**  Copyright (C) 2010-2014 Sergey Vlasov <sergey@vlasov.me>
**
**  This program can be distributed under the terms of the GNU
**  General Public License version 3.0 as published by the Free
**  Software Foundation and appearing in the file LICENSE.GPL3
**  included in the packaging of this file.  Please review the
**  following information to ensure the GNU General Public License
**  version 3.0 requirements will be met:
**
**  http://www.gnu.org/licenses/gpl-3.0.html
**
*********************************************************************/

#include "scriptEngine.h"

#include "global.h"
#include "scriptQtPrototypes.h"

#include "player.h"
#include "mainWindow.h"
#include "playbackEngineInterface.h"
#include "settings.h"

NWidgetPrototype widgetPrototype;
NLayoutPrototype layoutPrototype;
NSplitterPrototype splitterPrototype;

Q_DECLARE_METATYPE(QList<QWidget *>)

Q_DECLARE_METATYPE(NMainWindow *)
Q_DECLARE_METATYPE(NPlaybackEngineInterface *)
Q_DECLARE_METATYPE(N::PlaybackState)
Q_DECLARE_METATYPE(NSettings *)

template <typename T>
QScriptValue qObjectToScriptVlaue(QScriptEngine *engine, T const &obj)
{
	return engine->newQObject(obj);
}

template <typename T>
void qObjectFromScriptValue(const QScriptValue &value, T &obj)
{
	obj = qobject_cast<T>(value.toQObject());
}

struct QtMetaObject : private QObject
{
public:
	static const QMetaObject *get() { return &static_cast<QtMetaObject *>(0)->staticQtMetaObject; }
};

template <typename T>
QScriptValue enumToScriptValue(QScriptEngine *engine, T const &en)
{
	return engine->newVariant((int)en);
}

template <typename T>
void enumFromScriptValue(const QScriptValue &value, T &en)
{
	en = (T)value.toInt32();
}

QScriptValue readFile(QScriptContext *context, QScriptEngine *engine)
{
	QString path = context->argument(0).toString();
	QFile file(path);
	file.open(QFile::ReadOnly);
	return QLatin1String(file.readAll());
}

NScriptEngine::NScriptEngine(NPlayer *player) : QScriptEngine(player)
{
	QScriptValue global = globalObject();

	QScriptValue Qt = newQMetaObject(QtMetaObject::get());
	global.setProperty("Qt", Qt);

	global.setProperty("QT_VERSION", QT_VERSION);

	QString ws;
#if defined Q_WS_MAC
	ws = "mac";
#elif defined Q_WS_WIN
	ws = "win";
#elif defined Q_WS_X11
	ws = "x11";
#endif
	global.setProperty("Q_WS", ws);

	QString direction = "right";
	if (ws == "mac") {
		direction = "left";
	} else if (ws == "x11") {
		QProcess gconftool;
		gconftool.start("gconftool-2 --get \"/apps/metacity/general/button_layout\"");
		gconftool.waitForStarted();
		gconftool.waitForFinished();
		if (gconftool.readAll().endsWith(":\n"))
			direction = "left";
	}
	global.setProperty("WS_WM_BUTTON_DIRECTION", direction);

	qScriptRegisterMetaType(this, NMarginsPrototype::toScriptValue, NMarginsPrototype::fromScriptValue);
	setDefaultPrototype(qMetaTypeId<QWidget *>(), newQObject(&widgetPrototype));
	setDefaultPrototype(qMetaTypeId<QLayout *>(), newQObject(&layoutPrototype));
	setDefaultPrototype(qMetaTypeId<QSplitter *>(), newQObject(&splitterPrototype));
	qScriptRegisterSequenceMetaType< QList<QWidget *> >(this);

	qScriptRegisterMetaType<N::PlaybackState>(this, enumToScriptValue, enumFromScriptValue);
	QScriptValue N = newQMetaObject(&N::staticMetaObject);
	global.setProperty("N", N);

	qScriptRegisterMetaType<NMainWindow *>(this, qObjectToScriptVlaue, qObjectFromScriptValue);
	QScriptValue ui = newObject();
	global.setProperty("Ui", ui);
	NMainWindow *mainWindow = player->mainWindow();
	QList<QWidget *> widgets = mainWindow->findChildren<QWidget *>();
	foreach (QWidget *widget, widgets)
		ui.setProperty(widget->objectName(), newQObject(widget));
	ui.setProperty(mainWindow->objectName(), newQObject(mainWindow));

	qScriptRegisterMetaType<NPlaybackEngineInterface *>(this, qObjectToScriptVlaue, qObjectFromScriptValue);
	global.setProperty("PlaybackEngine", newQObject(player->playbackEngine()));

	qScriptRegisterMetaType<NSettings *>(this, qObjectToScriptVlaue, qObjectFromScriptValue);
	global.setProperty("Settings", newQObject(NSettings::instance()));

	globalObject().setProperty("readFile", newFunction(readFile));
}

