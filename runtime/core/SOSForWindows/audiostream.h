#pragma once
#ifndef AUDIOSTREAM_H
#define AUDIOSTREAM_H

#include <iostream>

#include <MMDeviceAPI.h>
#include <AudioClient.h>
#include <QDebug>
#include <iostream>
#include <stdio.h>
#include <Functiondiscoverykeys_devpkey.h>

#include "BufferUtil.h"

using namespace std;

class AudioStream {
signals:
  //void sendBuffer(BYTE*, size_t);
 public:

  AudioStream();
  AudioStream(BufferUtil * bu);
  BufferUtil* bufferUtil;

  //
  //  Write the contents of a WAV file.  We take as input the data to write and
  //  the format of that data.
  //
  bool WriteWaveFile(HANDLE FileHandle, const BYTE* Buffer,
                     const size_t BufferSize, const WAVEFORMATEX* WaveFormat);
  void SaveWaveData(BYTE* CaptureBuffer, size_t BufferSize,
                    const WAVEFORMATEX* WaveFormat, std::string Path);
  IMMDevice* GetDeviceByName(IMMDeviceCollection* pCollection,
                             const char* specifiedDeviceName);
  bool startCapture();
};





#endif // AUDIOSTREAM_H
