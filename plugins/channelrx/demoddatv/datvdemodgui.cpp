///////////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2018 F4HKW                                                      //
// for F4EXB / SDRAngel                                                          //
// using LeanSDR Framework (C) 2016 F4DAV                                        //
//                                                                               //
// This program is free software; you can redistribute it and/or modify          //
// it under the terms of the GNU General Public License as published by          //
// the Free Software Foundation as version 3 of the License, or                  //
//                                                                               //
// This program is distributed in the hope that it will be useful,               //
// but WITHOUT ANY WARRANTY; without even the implied warranty of                //
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the                  //
// GNU General Public License V3 for more details.                               //
//                                                                               //
// You should have received a copy of the GNU General Public License             //
// along with this program. If not, see <http://www.gnu.org/licenses/>.          //
///////////////////////////////////////////////////////////////////////////////////

#include <QDockWidget>
#include <QMainWindow>
#include <QMediaMetaData>

#include "datvdemodgui.h"
#include "datvideostream.h"

#include "device/devicesourceapi.h"
#include "device/deviceuiset.h"
#include "device/devicesourceapi.h"
#include "dsp/downchannelizer.h"

#include "dsp/threadedbasebandsamplesink.h"
#include "ui_datvdemodgui.h"
#include "plugin/pluginapi.h"
#include "util/simpleserializer.h"
#include "util/db.h"
#include "dsp/dspengine.h"
#include "mainwindow.h"

const QString DATVDemodGUI::m_strChannelID = "sdrangel.channel.demoddatv";

DATVDemodGUI* DATVDemodGUI::create(PluginAPI* objPluginAPI,
        DeviceUISet *deviceUISet,
        BasebandSampleSink *rxChannel)
{
    DATVDemodGUI* gui = new DATVDemodGUI(objPluginAPI, deviceUISet, rxChannel);
    return gui;
}

void DATVDemodGUI::destroy()
{    
    delete this;
}

void DATVDemodGUI::setName(const QString& strName)
{
    setObjectName(strName);
}

QString DATVDemodGUI::getName() const
{
    return objectName();
}

qint64 DATVDemodGUI::getCenterFrequency() const
{
    return m_objChannelMarker.getCenterFrequency();
}

void DATVDemodGUI::setCenterFrequency(qint64 intCenterFrequency)
{
    m_objChannelMarker.setCenterFrequency(intCenterFrequency);
    applySettings();
}

void DATVDemodGUI::resetToDefaults()
{
    blockApplySettings(true);

    ui->chkAllowDrift->setChecked(false);
    ui->chkFastlock->setChecked(true);
    ui->chkHDLC->setChecked(false);
    ui->chkHardMetric->setChecked(false);
    ui->chkResample->setChecked(false);
    ui->chkViterbi->setChecked(false);

    ui->cmbFEC->setCurrentIndex(0);
    ui->cmbModulation->setCurrentIndex(0);
    ui->cmbStandard->setCurrentIndex(0);

    ui->spiNotchFilters->setValue(1);
    ui->prgSynchro->setValue(0);

    ui->lblStatus->setText("");

    ui->spiBandwidth->setValue(512000);
    ui->spiSymbolRate->setValue(250000);

    blockApplySettings(false);

    applySettings();
}

QByteArray DATVDemodGUI::serialize() const
{
    SimpleSerializer s(1);

    s.writeS32(1, m_objChannelMarker.getCenterFrequency());
    s.writeU32(2, m_objChannelMarker.getColor().rgb());

    s.writeBool(3, ui->chkAllowDrift->isChecked());
    s.writeBool(4, ui->chkFastlock->isChecked());
    s.writeBool(5, ui->chkHDLC->isChecked());
    s.writeBool(6, ui->chkHardMetric->isChecked());
    s.writeBool(7, ui->chkResample->isChecked());
    s.writeBool(8, ui->chkViterbi->isChecked());

    s.writeS32(9, ui->cmbFEC->currentIndex());
    s.writeS32(10, ui->cmbModulation->currentIndex());
    s.writeS32(11, ui->cmbStandard->currentIndex());

    s.writeS32(12, ui->spiNotchFilters->value());
    s.writeS32(13, ui->spiBandwidth->value());
    s.writeS32(14, ui->spiSymbolRate->value());

    return s.final();
}

bool DATVDemodGUI::deserialize(const QByteArray& arrData)
{
    SimpleDeserializer d(arrData);

    if (!d.isValid())
    {
        resetToDefaults();
        return false;
    }

    if (d.getVersion() == 1)
    {
        QByteArray bytetmp;
        uint32_t u32tmp;
        int tmp;
        bool booltmp;

        blockApplySettings(true);
        m_objChannelMarker.blockSignals(true);

        d.readS32(1, &tmp, 0);
        m_objChannelMarker.setCenterFrequency(tmp);

        if (d.readU32(2, &u32tmp))
        {
            m_objChannelMarker.setColor(u32tmp);
        }
        else
        {
            m_objChannelMarker.setColor(Qt::magenta);
        }

        d.readBool(3, &booltmp, false);
        ui->chkAllowDrift->setChecked(booltmp);

        d.readBool(4, &booltmp, false);
        ui->chkFastlock->setChecked(booltmp);

        d.readBool(5, &booltmp, false);
        ui->chkHDLC->setChecked(booltmp);

        d.readBool(6, &booltmp, false);
        ui->chkHardMetric->setChecked(booltmp);

        d.readBool(7, &booltmp, false);
        ui->chkResample->setChecked(booltmp);

        d.readBool(8, &booltmp, false);
        ui->chkViterbi->setChecked(booltmp);


        d.readS32(9, &tmp, 0);
        ui->cmbFEC->setCurrentIndex(tmp);

        d.readS32(10, &tmp, 0);
        ui->cmbModulation->setCurrentIndex(tmp);

        d.readS32(11, &tmp, 0);
        ui->cmbStandard->setCurrentIndex(tmp);

        d.readS32(12, &tmp, 1);
        ui->spiNotchFilters->setValue(tmp);

        d.readS32(13, &tmp, 1024000);
        ui->spiBandwidth->setValue(tmp);

        d.readS32(14, &tmp, 250000);
        ui->spiSymbolRate->setValue(tmp);


        blockApplySettings(false);
        m_objChannelMarker.blockSignals(false);

        applySettings();
        return true;
    }
    else
    {
        resetToDefaults();
        return false;
    }
}

bool DATVDemodGUI::handleMessage(const Message& objMessage)
{
    return false;
}

void DATVDemodGUI::channelMarkerChangedByCursor()
{
    if(m_intCenterFrequency!=m_objChannelMarker.getCenterFrequency())
    {
        m_intCenterFrequency=m_objChannelMarker.getCenterFrequency();
        applySettings();
    }
}

void DATVDemodGUI::channelMarkerHighlightedByCursor()
{
    setHighlighted(m_objChannelMarker.getHighlighted());
}

void DATVDemodGUI::channelSampleRateChanged()
{
    qDebug("DATVDemodGUI::channelSampleRateChanged");
    applySettings();
}

void DATVDemodGUI::onWidgetRolled(QWidget* widget, bool rollDown)
{
}

void DATVDemodGUI::onMenuDoubleClicked()
{
    /*
    if (!m_blnBasicSettingsShown)
    {
        m_blnBasicSettingsShown = true;
        BasicChannelSettingsWidget* bcsw = new BasicChannelSettingsWidget(&m_objChannelMarker, this);
        bcsw->show();
    }
    */
}

//DATVDemodGUI::DATVDemodGUI(PluginAPI* objPluginAPI, DeviceSourceAPI *objDeviceAPI, QWidget* objParent) :
DATVDemodGUI::DATVDemodGUI(PluginAPI* objPluginAPI, DeviceUISet *deviceUISet, BasebandSampleSink *rxChannel, QWidget* objParent) :
        RollupWidget(objParent),
        ui(new Ui::DATVDemodGUI),
        m_objPluginAPI(objPluginAPI),
        m_deviceUISet(deviceUISet),
        m_objChannelMarker(this),
        m_blnBasicSettingsShown(false),
        m_blnDoApplySettings(true)
{    
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose, true);
    connect(this, SIGNAL(widgetRolled(QWidget*,bool)), this, SLOT(onWidgetRolled(QWidget*,bool)));
    //connect(this, SIGNAL(menuDoubleClickEvent()), this, SLOT(onMenuDoubleClicked()));

    //m_objDATVDemod = new DATVDemod();
    m_objDATVDemod = (DATVDemod*) rxChannel;
    m_objDATVDemod->setMessageQueueToGUI(getInputMessageQueue());

    m_objDATVDemod->SetDATVScreen(ui->screenTV);

    connect(m_objDATVDemod->SetVideoRender(ui->screenTV_2),&DATVideostream::onDataPackets,this,&DATVDemodGUI::on_StreamDataAvailable);


    //connect(m_objChannelizer, SIGNAL(inputSampleRateChanged()), this, SLOT(channelSampleRateChanged()));


    //m_objPluginAPI->addThreadedSink(m_objThreadedChannelizer);
    //connect(&m_objPluginAPI->getMainWindow()->getMasterTimer(), SIGNAL(timeout()), this, SLOT(tick())); // 50 ms

    m_intPreviousDecodedData=0;
    m_intLastDecodedData=0;
    m_intLastSpeed=0;
    m_intReadyDecodedData=0;
    m_blnButtonPlayClicked=false;
    m_objTimer.setInterval(1000);
    connect(&m_objTimer, SIGNAL(timeout()), this, SLOT(tick()));
    m_objTimer.start();


    m_objChannelMarker.blockSignals(true);
    m_objChannelMarker.setColor(Qt::magenta);
    m_objChannelMarker.setBandwidth(6000000);
    m_objChannelMarker.setCenterFrequency(0);
    m_objChannelMarker.blockSignals(false);
    m_objChannelMarker.setVisible(true);
    //connect(&m_objChannelMarker, SIGNAL(changed()), this, SLOT(viewChanged()));

    connect(&m_objChannelMarker, SIGNAL(changedByCursor()), this, SLOT(channelMarkerChangedByCursor()));
    connect(&m_objChannelMarker, SIGNAL(highlightedByCursor()), this, SLOT(channelMarkerHighlightedByCursor()));

    m_deviceUISet->registerRxChannelInstance(DATVDemod::m_channelIdURI, this);
    m_deviceUISet->addChannelMarker(&m_objChannelMarker);
    m_deviceUISet->addRollupWidget(this);

    //ui->screenTV->connectTimer(m_objPluginAPI->getMainWindow()->getMasterTimer());

    ui->pushButton_3->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));

    resetToDefaults(); // does applySettings()

}

DATVDemodGUI::~DATVDemodGUI()
{

    m_deviceUISet->removeRxChannelInstance(this);
    delete m_objDATVDemod;
    delete ui;

}

void DATVDemodGUI::blockApplySettings(bool blnBlock)
{
    m_blnDoApplySettings = !blnBlock;
}

void DATVDemodGUI::applySettings()
{
    QString strStandard;
    QString strModulation;
    QString strFEC;

    DATVModulation enmSelectedModulation;
    dvb_version enmVersion;
    code_rate enmFEC;

    if (m_blnDoApplySettings)
    {


        DATVDemod::MsgConfigureChannelizer *msgChan = DATVDemod::MsgConfigureChannelizer::create(m_objChannelMarker.getCenterFrequency());
        m_objDATVDemod->getInputMessageQueue()->push(msgChan);

        //Bandwidth and  center frequency
        m_objChannelMarker.setBandwidth(ui->spiBandwidth->value());
        //m_objChannelizer->configure(m_objChannelizer->getInputMessageQueue(), m_objChannelizer->getInputSampleRate(), m_objChannelMarker.getCenterFrequency());

        setTitleColor(m_objChannelMarker.getColor());

        //DATV parameters: cmbStandard  cmbModulation cmbFEC spiBandwidth spiSymbolRate spiNotchFilters chkAllowDrift chkFastlock chkHDLC chkHardMetric chkResample chkViterbi

        strStandard = ui->cmbStandard->currentText();

        if(strStandard=="DVB-S")
        {
            enmVersion=DVB_S;
        }
        else if (strStandard=="DVB-S2")
        {
            enmVersion=DVB_S2;
        }
        else
        {
            enmVersion=DVB_S;
        }


        //BPSK, QPSK, PSK8, APSK16, APSK32, APSK64E, QAM16, QAM64, QAM256

        strModulation = ui->cmbModulation->currentText();

        if(strModulation=="BPSK")
        {
            enmSelectedModulation=BPSK;
        }
        else if(strModulation=="QPSK")
        {
            enmSelectedModulation=QPSK;
        }
        else if(strModulation=="8PSK")
        {
            enmSelectedModulation=PSK8;
        }
        else if(strModulation=="16APSK")
        {
            enmSelectedModulation=APSK16;
        }
        else if(strModulation=="32APSK")
        {
            enmSelectedModulation=APSK32;
        }
        else if(strModulation=="64APSKE")
        {
            enmSelectedModulation=APSK64E;
        }
        else if(strModulation=="16QAM")
        {
            enmSelectedModulation=QAM16;
        }
        else if(strModulation=="64QAM")
        {
            enmSelectedModulation=QAM64;
        }
        else if(strModulation=="256QAM")
        {
            enmSelectedModulation=QAM256;
        }
        else
        {
            enmSelectedModulation=BPSK;
        }


        strFEC = ui->cmbFEC->currentText();

        if(strFEC=="1/2")
        {
            enmFEC=FEC12;
        }
        else if(strFEC=="2/3")
        {
            enmFEC=FEC23;
        }
        else if(strFEC=="3/4")
        {
            enmFEC=FEC34;
        }
        else if(strFEC=="5/6")
        {
            enmFEC=FEC56;
        }
        else if(strFEC=="7/8")
        {
            enmFEC=FEC78;
        }
        else if(strFEC=="4/5")
        {
            enmFEC=FEC45;
        }
        else if(strFEC=="8/9")
        {
            enmFEC=FEC89;
        }
        else if(strFEC=="9/10")
        {
            enmFEC=FEC910;
        }
        else
        {
            enmFEC=FEC12;
        }


        m_objDATVDemod->configure(m_objDATVDemod->getInputMessageQueue(),
                                  m_objChannelMarker.getBandwidth(),
                                  m_objChannelMarker.getCenterFrequency(),
                                  enmVersion,
                                  enmSelectedModulation,
                                  enmFEC,
                                  ui->spiSymbolRate->value(),
                                  ui->spiNotchFilters->value(),
                                  ui->chkAllowDrift->isChecked(),
                                  ui->chkFastlock->isChecked(),
                                  ui->chkHDLC->isChecked(),
                                  ui->chkHardMetric->isChecked(),
                                  ui->chkResample->isChecked(),
                                  ui->chkViterbi->isChecked());

        qDebug() << "DATVDemodGUI::applySettings:"
                << " .inputSampleRate: " << 0  /*m_objChannelizer->getInputSampleRate()*/
                << " m_objDATVDemod.sampleRate: " << m_objDATVDemod->GetSampleRate();


    }
}

void DATVDemodGUI::leaveEvent(QEvent*)
{
    blockApplySettings(true);
    m_objChannelMarker.setHighlighted(false);
    blockApplySettings(false);
}

void DATVDemodGUI::enterEvent(QEvent*)
{
    blockApplySettings(true);
    m_objChannelMarker.setHighlighted(true);
    blockApplySettings(false);
}

void DATVDemodGUI::tick()
{     
    if((m_intLastDecodedData-m_intPreviousDecodedData)>=0)
    {        
        m_intLastSpeed = 8*(m_intLastDecodedData-m_intPreviousDecodedData);
        ui->lblRate->setText(QString("Speed: %1b/s").arg(formatBytes(m_intLastSpeed)));
    }

    m_intPreviousDecodedData = m_intLastDecodedData;

    //Try to start video rendering
    m_objDATVDemod->PlayVideo(false);

    return;
}

void DATVDemodGUI::on_cmbStandard_currentIndexChanged(const QString &arg1)
{   
    applySettings();
}

void DATVDemodGUI::on_cmbModulation_currentIndexChanged(const QString &arg1)
{
    QString strModulation;
    QString strFEC;

    strFEC = ui->cmbFEC->currentText();

    strModulation = ui->cmbModulation->currentText();

    if(strModulation=="16APSK")
    {
        if((strFEC!="2/3")
            && (strFEC!="3/4")
            && (strFEC!="4/5")
            && (strFEC!="5/6")
            && (strFEC!="4/6")
            && (strFEC!="8/9")
            && (strFEC!="9/10"))
        {
            //Reset modulation to 2/3

            ui->cmbFEC->setCurrentIndex(1);
        }
        else
        {
            applySettings();
        }
    }
    else if(strModulation=="32APSK")
    {
        if((strFEC!="3/4")
            && (strFEC!="4/5")
            && (strFEC!="5/6")
            && (strFEC!="8/9")
            && (strFEC!="9/10"))
        {
            //Reset modulation to 3/4

            ui->cmbFEC->setCurrentIndex(2);
        }
        else
        {
            applySettings();
        }
    }
    else
    {
        applySettings();
    }

}

void DATVDemodGUI::on_cmbFEC_currentIndexChanged(const QString &arg1)
{    
    QString strFEC;

    strFEC = ui->cmbFEC->currentText();

    if(ui->cmbModulation->currentText()=="16APSK")
    {
        if((strFEC!="2/3")
            && (strFEC!="3/4")
            && (strFEC!="4/5")
            && (strFEC!="5/6")
            && (strFEC!="4/6")
            && (strFEC!="8/9")
            && (strFEC!="9/10"))
        {
            //Reset modulation to BPSK

            ui->cmbModulation->setCurrentIndex(0);
        }
        else
        {
            applySettings();
        }
    }
    else if(ui->cmbModulation->currentText()=="32APSK")
    {
        if((strFEC!="3/4")
            && (strFEC!="4/5")
            && (strFEC!="5/6")
            && (strFEC!="8/9")
            && (strFEC!="9/10"))
        {
            //Reset modulation to BPSK

            ui->cmbModulation->setCurrentIndex(0);
        }
        else
        {
            applySettings();
        }
    }
    else
    {
        applySettings();
    }

}

void DATVDemodGUI::on_chkViterbi_clicked()
{
    applySettings();
}

void DATVDemodGUI::on_chkHardMetric_clicked()
{
    applySettings();
}

/*
void DATVDemodGUI::on_pushButton_clicked()
{
    applySettings();    
}
*/

void DATVDemodGUI::on_pushButton_2_clicked()
{
    resetToDefaults();
}

/*
void DATVDemodGUI::on_spiSampleRate_valueChanged(int arg1)
{
    applySettings();
}
*/

void DATVDemodGUI::on_spiSymbolRate_valueChanged(int arg1)
{
    applySettings();
}

void DATVDemodGUI::on_spiNotchFilters_valueChanged(int arg1)
{
    applySettings();
}

void DATVDemodGUI::on_chkHDLC_clicked()
{
     applySettings();
}

void DATVDemodGUI::on_chkAllowDrift_clicked()
{
     applySettings();
}

void DATVDemodGUI::on_chkResample_clicked()
{
     applySettings();
}

void DATVDemodGUI::on_pushButton_3_clicked()
{

   m_blnButtonPlayClicked=true;

   if(m_objDATVDemod!=NULL)
   {
       m_objDATVDemod->PlayVideo(true);
   }
}

/*
void DATVDemodGUI::on_mediaStateChanged(QMediaPlayer::State state)
{
    switch(state)
    {
        case QMediaPlayer::PlayingState:
            ui->pushButton_3->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
            break;
        default:
            ui->pushButton_3->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
            break;
    }

    ui->lblReadStatus->setText(QString("%1").arg(strLitteralState));

}
*/

void DATVDemodGUI::on_pushButton_4_clicked()
{
    ui->screenTV_2->SetFullScreen(true);

}

void DATVDemodGUI::on_mouseEvent(QMouseEvent* obj)
{    
}

QString DATVDemodGUI::formatBytes(qint64 intBytes)
{
    if(intBytes<1024)
    {
        return QString("%1").arg(intBytes);
    }
    else if(intBytes<1024*1024)
    {
        return QString("%1 K").arg((float)(10*intBytes/1024)/10.0f);
    }
    else if(intBytes<1024*1024*1024)
    {
        return QString("%1 M").arg((float)(10*intBytes/(1024*1024))/10.0f);
    }

    return QString("%1 G").arg((float)(10*intBytes/(1024*1024*1024))/10.0f);
}


void DATVDemodGUI::on_StreamDataAvailable(int *intPackets, int *intBytes, int *intPercent, qint64 *intTotalReceived)
{
    ui->lblStatus->setText(QString("Decod: %1B").arg(formatBytes(*intTotalReceived)));
    m_intLastDecodedData = *intTotalReceived;

    if((*intPercent)<100)
    {
         ui->prgSynchro->setValue(*intPercent);
    }
    else
    {
         ui->prgSynchro->setValue(100);
    }

    m_intReadyDecodedData = *intBytes;

}

void DATVDemodGUI::on_spiBandwidth_valueChanged(int arg1)
{
    applySettings();
}


void DATVDemodGUI::on_chkFastlock_clicked()
{
    applySettings();
}
