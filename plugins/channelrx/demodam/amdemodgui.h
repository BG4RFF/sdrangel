#ifndef INCLUDE_AMDEMODGUI_H
#define INCLUDE_AMDEMODGUI_H

#include "gui/rollupwidget.h"
#include "plugin/plugingui.h"
#include "dsp/channelmarker.h"
#include "dsp/movingaverage.h"

class PluginAPI;
class DeviceAPI;

class ThreadedSampleSink;
class DownChannelizer;
class AMDemod;

namespace Ui {
	class AMDemodGUI;
}

class AMDemodGUI : public RollupWidget, public PluginGUI {
	Q_OBJECT

public:
	static AMDemodGUI* create(PluginAPI* pluginAPI, DeviceAPI *deviceAPI);
	void destroy();

	void setName(const QString& name);
	QString getName() const;
	virtual qint64 getCenterFrequency() const;
	virtual void setCenterFrequency(qint64 centerFrequency);

	void resetToDefaults();
	QByteArray serialize() const;
	bool deserialize(const QByteArray& data);

	virtual bool handleMessage(const Message& message);

	static const QString m_channelID;

private slots:
	void viewChanged();
	void on_deltaFrequency_changed(quint64 value);
	void on_deltaMinus_toggled(bool minus);
	void on_rfBW_valueChanged(int value);
	void on_afBW_valueChanged(int value);
	void on_volume_valueChanged(int value);
	void on_squelch_valueChanged(int value);
	void on_audioMute_toggled(bool checked);
	void onWidgetRolled(QWidget* widget, bool rollDown);
	void onMenuDoubleClicked();
	void tick();

private:
	Ui::AMDemodGUI* ui;
	PluginAPI* m_pluginAPI;
	DeviceAPI* m_deviceAPI;
	ChannelMarker m_channelMarker;
	bool m_basicSettingsShown;
	bool m_doApplySettings;

	ThreadedSampleSink* m_threadedChannelizer;
	DownChannelizer* m_channelizer;
	AMDemod* m_amDemod;
	MovingAverage<Real> m_channelPowerDbAvg;
	bool m_squelchOpen;

	static const int m_rfBW[];

	explicit AMDemodGUI(PluginAPI* pluginAPI, DeviceAPI *deviceAPI, QWidget* parent = NULL);
	virtual ~AMDemodGUI();

    void blockApplySettings(bool block);
	void applySettings();

	void leaveEvent(QEvent*);
	void enterEvent(QEvent*);
};

#endif // INCLUDE_AMDEMODGUI_H