///////////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2018 F4EXB                                                      //
// written by Edouard Griffiths                                                  //
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

#ifndef SDRBASE_AUDIO_AUDIONETSINK_H_
#define SDRBASE_AUDIO_AUDIONETSINK_H_

#include <QObject>
#include "dsp/dsptypes.h"
#include "util/export.h"

template<typename T> class UDPSink;
class RTPSink;

class SDRANGEL_API AudioNetSink {
public:
    typedef enum
    {
        SinkUDP,
        SinkRTP
    } SinkType;

    AudioNetSink(QObject *parent, bool stereo = false);
    ~AudioNetSink();

    void setDestination(const QString& address, uint16_t port);
    void addDestination(const QString& address, uint16_t port);
    void deleteDestination(const QString& address, uint16_t port);

    void write(qint16 sample);
    void write(const AudioSample& sample);

    bool isRTPCapable() const;
    bool selectType(SinkType type);

    static const int m_udpBlockSize;

protected:
    SinkType m_type;
    UDPSink<qint16> *m_udpBufferAudioMono;
    UDPSink<AudioSample> *m_udpBufferAudioStereo;
    RTPSink *m_rtpBufferAudio;
};




#endif /* SDRBASE_AUDIO_AUDIONETSINK_H_ */
