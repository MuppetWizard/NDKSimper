//
// Created by ASUS on 2022/4/22.
//

#include "MPlayer.h"
#include <unistd.h>

MPlayer::MPlayer() {
}

MPlayer::MPlayer(const char *dataSource, JNICallback *pCallback) {
    this->data_source = new char[strlen(dataSource) + 1];
    strcpy(this->data_source,dataSource);
    this->pCallback = pCallback;
    duration = 0;
//    pthread_mutex_init
}

MPlayer::~MPlayer() {

}

void MPlayer::prepare() {

}
