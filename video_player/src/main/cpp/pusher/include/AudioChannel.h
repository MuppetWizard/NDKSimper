//
// Created by ASUS on 2022/5/7.
//

#ifndef NDKSIMPLE_AUDIOCHANNEL_H
#define NDKSIMPLE_AUDIOCHANNEL_H

#define FAAC_DEFAUTE_SAMPLE_RATE 44100
#define FAAC_DEFAUTE_SAMPLE_CHANNEL 1

#include <rtmp.h>
#include <sys/types.h>
//#include <faac.h>

class AudioChannel {
    typedef void (*AudioCallback)(RTMPPacket *packet);

};


#endif //NDKSIMPLE_AUDIOCHANNEL_H
