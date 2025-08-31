/*
模块名称：音频处理模块
实现功能：录制音频，处理音频数据
完成作者：樊泽瑞
测试人员：郭明皓 樊泽瑞
审核人员：郭明皓 刘元庆
*/
#include "audiostream.h"

#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif


// ns(nanosecond) : 纳秒，时间单位。一秒的十亿分之一
// 1秒=1000毫秒; 1毫秒=1000微秒; 1微秒=1000纳秒

// The REFERENCE_TIME data type defines the units for reference times in DirectShow.
// Each unit of reference time is 100 nanoseconds.(100纳秒为一个REFERENCE_TIME时间单位)

// REFERENCE_TIME time units per second and per millisecond
#define REFTIMES_PER_SEC       (10000000)
#define REFTIMES_PER_MILLISEC  (10000)

#define EXIT_ON_ERROR(hres)  \
    if (FAILED(hres)) { goto Exit; }

#define SAFE_RELEASE(punk)  \
    if ((punk) != NULL)  \
                { (punk)->Release(); (punk) = NULL; }

const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
const IID   IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
const IID   IID_IAudioClient = __uuidof(IAudioClient);
const IID   IID_IAudioCaptureClient = __uuidof(IAudioCaptureClient);

#define MoveMemory RtlMoveMemory
#define CopyMemory RtlCopyMemory
#define FillMemory RtlFillMemory
#define ZeroMemory RtlZeroMemory

#define min(a,b)            (((a) < (b)) ? (a) : (b))

// Global vars
HRESULT hr;

IMMDeviceEnumerator* pEnumerator = NULL;
IMMDevice* pDevice = NULL;
IAudioClient* pAudioClient = NULL;
IAudioCaptureClient* pCaptureClient = NULL;
WAVEFORMATEX* pwfx = NULL;

REFERENCE_TIME hnsRequestedDuration = REFTIMES_PER_SEC;
UINT32         bufferFrameCount;
UINT32         numFramesAvailable;

BYTE* pData;
UINT32         packetLength = 0;
DWORD          flags;

REFERENCE_TIME hnsActualDuration;
IMMDeviceCollection* pCollection = NULL;

BYTE* pbyCaptureBuffer;
//
//  A wave file consists of:
//
//  RIFF header:    8 bytes consisting of the signature "RIFF" followed by a 4 byte file length.
//  WAVE header:    4 bytes consisting of the signature "WAVE".
//  fmt header:     4 bytes consisting of the signature "fmt " followed by a WAVEFORMATEX
//  WAVEFORMAT:     <n> bytes containing a waveformat structure.
//  DATA header:    8 bytes consisting of the signature "data" followed by a 4 byte file length.
//  wave data:      <m> bytes containing wave data.
//
//
//  Header for a WAV file - we define a structure describing the first few fields
// in the header for convenience.
//
struct WAVEHEADER
{
    DWORD   dwRiff;                     // "RIFF"
    DWORD   dwSize;                     // Size
    DWORD   dwWave;                     // "WAVE"
    DWORD   dwFmt;                      // "fmt "
    DWORD   dwFmtSize;                  // Wave Format Size
};

//  Static RIFF header, we'll append the format to it.
const BYTE WaveHeader[] =
{
    'R',   'I',   'F',   'F',  0x00,  0x00,  0x00,  0x00, 'W',   'A',   'V',   'E',   'f',   'm',   't',   ' ', 0x00, 0x00, 0x00, 0x00
};

//  Static wave DATA tag.
const BYTE WaveData[] = { 'd', 'a', 't', 'a' };

#include <cstring>
#include <cmath>

void pcm_downsample(
    const BYTE* Buffer, 
    const size_t BufferSize,
    const WAVEFORMATEX* WaveFormat, 
    BYTE* OutBuffer, 
    size_t* nBufferSize
) {
    // 输入校验
    if (!Buffer || !OutBuffer || !WaveFormat || !nBufferSize || BufferSize == 0) {
        if (nBufferSize) *nBufferSize = 0;
        return;
    }

    // 只支持 32-bit float PCM 输入
    if (WaveFormat->wBitsPerSample != 32 || WaveFormat->wFormatTag != WAVE_FORMAT_IEEE_FLOAT) {
        *nBufferSize = 0;
        return;
    }

    // 假设单通道（Mono），多通道需额外处理
    if (WaveFormat->nChannels != 1) {
        *nBufferSize = 0;
        return;
    }

    const int srcSampleRate = WaveFormat->nSamplesPerSec;
    const int dstSampleRate = 16000;  // 目标采样率

    // 计算降采样比例（浮点以支持非整数比）
    const float ratio = static_cast<float>(srcSampleRate) / dstSampleRate;

    // 每个样本 4 字节 (32-bit float)
    const size_t sampleSize = sizeof(float);
    const size_t numInputSamples = BufferSize / sampleSize;

    float accumulator = 0.0f;
    int count = 0;
    size_t outIndex = 0;

    for (size_t i = 0; i < numInputSamples; ++i) {
        float sample;
        memcpy(&sample, Buffer + i * sampleSize, sampleSize);

        accumulator += sample;
        count++;

        // 当累计足够样本对应一个输出样本时
        if (count >= std::round(ratio)) {
            // 求平均（抗混叠）
            float avg_sample = accumulator / count;

            // 转为 16-bit signed integer，注意范围 [-1.0, 1.0] -> [-32768, 32767]
            // 32-bit float PCM 通常归一化在 [-1.0, 1.0]
            short out_sample = static_cast<short>(std::clamp(
                avg_sample * 32767.0f,
                -32768.0f,
                32767.0f
            ));

            // 写入输出缓冲区（小端字节序）
            memcpy(OutBuffer + outIndex, &out_sample, sizeof(short));
            outIndex += sizeof(short);

            // 重置累加器
            accumulator = 0.0f;
            count = 0;
        }
    }

    *nBufferSize = outIndex;
}

AudioStream::AudioStream() {}

AudioStream::AudioStream(BufferUtil* bu) { this->bufferUtil = bu; }

bool AudioStream::WriteWaveFile(HANDLE FileHandle, const BYTE* Buffer,
                              const size_t BufferSize,
                              const WAVEFORMATEX* WaveFormat) {
  BYTE* OutBuffer = new BYTE[BufferSize * 16000 / WaveFormat->nSamplesPerSec];
  size_t nBufferSize = BufferSize * 16000 / WaveFormat->nSamplesPerSec;
  pcm_downsample(Buffer,  BufferSize,
                 WaveFormat, OutBuffer,&nBufferSize);
  
    DWORD waveFileSize = sizeof(WAVEHEADER) + sizeof(WAVEFORMATEX) + WaveFormat->cbSize + sizeof(WaveData) + sizeof(DWORD) + static_cast<DWORD>(nBufferSize);
    BYTE* waveFileData = new (std::nothrow) BYTE[waveFileSize];
    BYTE* waveFilePointer = waveFileData;
    WAVEHEADER* waveHeader = reinterpret_cast<WAVEHEADER*>(waveFileData);

    if (waveFileData == NULL)
    {
        printf("Unable to allocate %d bytes to hold output wave data\n", waveFileSize);
        return false;
    }

    //
    //  Copy in the wave header - we'll fix up the lengths later.
    //
    CopyMemory(waveFilePointer, WaveHeader, sizeof(WaveHeader));
    waveFilePointer += sizeof(WaveHeader);

    //
    //  Update the sizes in the header.
    //
    waveHeader->dwSize = waveFileSize - (2 * sizeof(DWORD));
    waveHeader->dwFmtSize = sizeof(WAVEFORMATEX) + WaveFormat->cbSize;

    //
    //  Next copy in the WaveFormatex structure.
    //
    
    CopyMemory(waveFilePointer, WaveFormat, sizeof(WAVEFORMATEX) + WaveFormat->cbSize);
    waveFilePointer += sizeof(WAVEFORMATEX) + WaveFormat->cbSize;


    //
    //  Then the data header.
    //
    CopyMemory(waveFilePointer, WaveData, sizeof(WaveData));
    waveFilePointer += sizeof(WaveData);
    *(reinterpret_cast<DWORD*>(waveFilePointer)) = static_cast<DWORD>(nBufferSize);
    waveFilePointer += sizeof(DWORD);

    //
    //  And finally copy in the audio data.
    //
    CopyMemory(waveFilePointer, OutBuffer, nBufferSize);
    //CopyMemory(waveFilePointer, Buffer, BufferSize);
    //
    //  Last but not least, write the data to the file.
    //
    DWORD bytesWritten;
    if (!WriteFile(FileHandle, waveFileData, waveFileSize, &bytesWritten, NULL))
    {
        printf("Unable to write wave file: %d\n", GetLastError());
        delete[]waveFileData;
        return false;
    }

    if (bytesWritten != waveFileSize)
    {
        printf("Failed to write entire wave file\n");
        delete[]waveFileData;
        return false;
    }
    delete[]waveFileData;
    return true;
}

//
//  Write the captured wave data to an output file so that it can be examined later.
//
void AudioStream::SaveWaveData(BYTE* CaptureBuffer, size_t BufferSize,
                             const WAVEFORMATEX* WaveFormat, std::string Path) {
    HRESULT hr = NOERROR;

    SYSTEMTIME st;
    GetLocalTime(&st);
    LPCSTR waveFileName;
    if (Path != "") {
      char cache_path[1024];
      strcpy(cache_path, (Path + "\\cache.wav").c_str());
      waveFileName = (LPCSTR)(cache_path);
    } else {
      waveFileName = (LPCSTR)".\\cache.wav";
    }
    //sprintf(waveFileName, ".\\WAS_%04d-%02d-%02d_%02d_%02d_%02d_%02d.wav", st.wYear, st.wMonth, st.wDay,
    //	st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);

    HANDLE waveHandle = CreateFile(waveFileName, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
        NULL);
    if (waveHandle != INVALID_HANDLE_VALUE)
    {
        if (WriteWaveFile(waveHandle, CaptureBuffer, BufferSize, WaveFormat))
        {
            printf("Successfully wrote WAVE data to %s\n", waveFileName);
        }
        else
        {
            printf("Unable to write wave file\n");
        }
        CloseHandle(waveHandle);
    }
    else
    {
        printf("Unable to open output WAV file %s: %d\n", waveFileName, GetLastError());
    }

}

IMMDevice* AudioStream::GetDeviceByName(IMMDeviceCollection* pCollection,
    const char* specifiedDeviceName) {
    HRESULT hr = S_OK;
    IMMDevice* pEndpoint = NULL;
    IPropertyStore* pProps = NULL;
    LPWSTR pwszID = NULL;
    IMMDevice* pSpecifiedDevice = NULL;
    ULONG specifiedDeviceIndex = -1;
    // 获取集合中端点对象的数量
    UINT  count;
    hr = pCollection->GetCount(&count);
    if (count == 0)
    {
        return NULL;
    }
    for (ULONG i = 0; i < count; i++)
    {
        // 获取集合中第i个端点对象的IMMDevice接口的引用
        hr = pCollection->Item(i, &pEndpoint);
        EXIT_ON_ERROR(hr)
            // 获取端点设备的ID字符串
            hr = pEndpoint->GetId(&pwszID);
        EXIT_ON_ERROR(hr)
            hr = pEndpoint->OpenPropertyStore(STGM_READ, &pProps);
        EXIT_ON_ERROR(hr)
            PROPVARIANT varName;
        // Initialize container for property value.
        PropVariantInit(&varName);
        // 获取端点设备的friendly-name属性
        hr = pProps->GetValue(PKEY_Device_FriendlyName, &varName);
        EXIT_ON_ERROR(hr)
            // 输出设备的名称
            wcout.imbue(locale("", LC_CTYPE));
        wcout << "wcout varName.pwszVal:" << varName.pwszVal << endl;

        //宽字符串转换为多字节字符串
        int bufSize = WideCharToMultiByte(CP_ACP, NULL, varName.pwszVal, -1, NULL, 0, NULL, FALSE);
        char* varName_pwszVal_str = new char[bufSize];
        WideCharToMultiByte(CP_ACP, NULL, varName.pwszVal, -1, varName_pwszVal_str, bufSize, NULL, FALSE);
        // 将指定的设备名称与遍历的名称比较
        // 若相同，则获取集合中设备对象所在的索引，后面将根据该下表获取指定的设备对象
        if (strcmp(specifiedDeviceName, varName_pwszVal_str) == 0) {
            specifiedDeviceIndex = i;
            break;
        }
        else {
            specifiedDeviceIndex = -1;
        }
        CoTaskMemFree(pwszID);
        pwszID = NULL;
        PropVariantClear(&varName);
        SAFE_RELEASE(pProps)
            SAFE_RELEASE(pEndpoint)
    }
    // 获取指定索引的端点设备对象
    if (specifiedDeviceIndex >= 0 && specifiedDeviceIndex < count)
    {
        hr = pCollection->Item(specifiedDeviceIndex, &pSpecifiedDevice);
        EXIT_ON_ERROR(hr)
    }
    else {
        return NULL;
    }
    // if specifiedDeviceIndex < 0, return NULL
Exit:
    CoTaskMemFree(pwszID);
    SAFE_RELEASE(pEndpoint)
        SAFE_RELEASE(pProps)

        cout << "GetDeviceByName specifiedDeviceIndex= " << specifiedDeviceIndex << endl;
    return pSpecifiedDevice;
}


/*
     设置AUDCLNT_STREAMFLAGS_LOOPBACK
     这种模式下，音频engine会将rending设备正在播放的音频流， 拷贝一份到音频的endpoint buffer
     这样的话，WASAPI client可以采集到the stream.
     此时仅采集到Speaker的声音
*/

bool AudioStream::startCapture() {
    qDebug() << "Thread Start...";
    hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if (FAILED(hr))
    {
        printf("Unable to initialize COM in thread: %x\n", hr);
        return hr;
    }

    // 首先枚举音频设备
    // 可以获取到机器上所有可用的设备，并指定需要用到的那个设备
    hr = CoCreateInstance(CLSID_MMDeviceEnumerator,
        NULL,
        CLSCTX_ALL,
        IID_IMMDeviceEnumerator,
        (void**)&pEnumerator);
    if (FAILED(hr)) {
        CoTaskMemFree(pwfx);
        SAFE_RELEASE(pEnumerator)
            SAFE_RELEASE(pDevice)
            SAFE_RELEASE(pAudioClient)
            SAFE_RELEASE(pCaptureClient)
            SAFE_RELEASE(pCollection)


            CoUninitialize();

        if (pbyCaptureBuffer)
        {
            delete[] pbyCaptureBuffer;
            pbyCaptureBuffer = NULL;
        }

        getchar();

        return 0;
    }

        // 获取默认的呈现终端设备
        hr = pEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &pDevice);
        if (FAILED(hr)) {
            CoTaskMemFree(pwfx);
            SAFE_RELEASE(pEnumerator)
                SAFE_RELEASE(pDevice)
                SAFE_RELEASE(pAudioClient)
                SAFE_RELEASE(pCaptureClient)
                SAFE_RELEASE(pCollection)


                CoUninitialize();

            if (pbyCaptureBuffer)
            {
                delete[] pbyCaptureBuffer;
                pbyCaptureBuffer = NULL;
            }

            getchar();

            return 0;
        }

        /*
        枚举所有呈现音频端点设备，并根据设备的名称指定音频端点设备
        */
        ///

        /*hr = pEnumerator->EnumAudioEndpoints(
            eRender, DEVICE_STATE_ACTIVE,
            &pCollection);
        if (FAILED(hr)) {
            CoTaskMemFree(pwfx);
            SAFE_RELEASE(pEnumerator)
                SAFE_RELEASE(pDevice)
                SAFE_RELEASE(pAudioClient)
                SAFE_RELEASE(pCaptureClient)
                SAFE_RELEASE(pCollection)


                CoUninitialize();

            if (pbyCaptureBuffer)
            {
                delete[] pbyCaptureBuffer;
                pbyCaptureBuffer = NULL;
            }

            getchar();

            return 0;
        }

        const char* HDMI_str = "HDMI (英特尔(R) 显示器音频)";
    const char* system_speaker_str = "扬声器 (Realtek(R) Audio)";
    pDevice = GetDeviceByName(pCollection, system_speaker_str);
    cout << "\nspecifiedDevice = " << pDevice << endl;*/

    // 创建具有指定接口的音频客户端对象
    // 创建一个管理对象，通过它可以获取到你需要的一切数据
    hr = pDevice->Activate(IID_IAudioClient, CLSCTX_ALL, NULL, (void**)&pAudioClient);
    if (FAILED(hr)) {
        CoTaskMemFree(pwfx);
        SAFE_RELEASE(pEnumerator)
            SAFE_RELEASE(pDevice)
            SAFE_RELEASE(pAudioClient)
            SAFE_RELEASE(pCaptureClient)
            SAFE_RELEASE(pCollection)


            CoUninitialize();

        if (pbyCaptureBuffer)
        {
            delete[] pbyCaptureBuffer;
            pbyCaptureBuffer = NULL;
        }

        getchar();

        return 0;
    }

        // 获取默认的音频数据格式
        hr = pAudioClient->GetMixFormat(&pwfx);
        if (FAILED(hr)) {
            CoTaskMemFree(pwfx);
            SAFE_RELEASE(pEnumerator)
                SAFE_RELEASE(pDevice)
                SAFE_RELEASE(pAudioClient)
                SAFE_RELEASE(pCaptureClient)
                SAFE_RELEASE(pCollection)


                CoUninitialize();

            if (pbyCaptureBuffer)
            {
                delete[] pbyCaptureBuffer;
                pbyCaptureBuffer = NULL;
            }

            getchar();

            return 0;
        }

        
        //typedef struct tWAVEFORMATEX
        //{
        //    WORD        wFormatTag;         // format type
        //    WORD        nChannels;          // number of channels (i.e. mono, stereo...)
        //    DWORD       nSamplesPerSec;     // sample rate
        //    DWORD       nAvgBytesPerSec;    // for buffer estimation
        //    WORD        nBlockAlign;        // block size of data
        //    WORD        wBitsPerSample;     // number of bits per sample of mono data
        //    WORD        cbSize;             // the count in bytes of the size of  extra information (after cbSize)
        //} WAVEFORMATEX;
        printf("\nGetMixFormat...\n");
    cout << "wFormatTag      : " << pwfx->wFormatTag << endl
        << "nChannels       : " << pwfx->nChannels << endl
        << "nSamplesPerSec  : " << pwfx->nSamplesPerSec << endl
        << "nAvgBytesPerSec : " << pwfx->nAvgBytesPerSec << endl
        << "nBlockAlign     : " << pwfx->nBlockAlign << endl
        << "wBitsPerSample  : " << pwfx->wBitsPerSample << endl
        << "cbSize          : " << pwfx->cbSize << endl << endl;

    // test for IsFormatSupported
    //
    /*实际上获取到的默认格式并不一定符合客户端所需要的设备格式参数，那么会遍历通道数、采样率，
     调用 IAudioClient 的 IsFormatSupported 接口查询出最适合的设备格式参数*/
    WAVEFORMATEX* wf;
    hr = pAudioClient->IsFormatSupported(AUDCLNT_SHAREMODE_SHARED, pwfx, &wf);
    if (FAILED(hr))
    {
        printf("IsFormatSupported fail.\n");
    }
    else
    {
        printf("\nIsFormatSupported.\n\n");
    }
    /*printf("\IsFormatSupported...\n");
    cout<<"wFormatTag      : "<<wf->wFormatTag<<endl
        <<"nChannels       : "<<wf->nChannels<<endl
        <<"nSamplesPerSec  : "<<wf->nSamplesPerSec<<endl
        <<"nAvgBytesPerSec : "<<wf->nAvgBytesPerSec<<endl
        <<"nBlockAlign     : "<<wf->nBlockAlign<<endl
        <<"wBitsPerSample  : "<<wf->wBitsPerSample<<endl
        <<"cbSize          : "<<wf->cbSize<<endl<<endl;*/
        //

    int nFrameSize = (pwfx->wBitsPerSample / 8) * pwfx->nChannels;

    cout << "nFrameSize           : " << nFrameSize << " Bytes" << endl
        << "hnsRequestedDuration : " << hnsRequestedDuration
        << " REFERENCE_TIME time units. 即(" << hnsRequestedDuration / 10000 << "ms)" << endl;

    // 初始化管理对象，在这里，你可以指定它的最大缓冲区长度，这个很重要，
    // 应用程序控制数据块的大小以及延时长短都靠这里的初始化，具体参数文档有解释
    // https://msdn.microsoft.com/en-us/library/dd370875(v=vs.85).aspx

    /*
    采集立体声混音： 希望同时采集本机声卡上的默认麦克风和默认render的数据
    这种模式下，音频engine会将rending设备正在播放的音频流， 拷贝一份到音频的endpoint buffer
    这样的话，WASAPI client可以采集到the stream.
    如果AUDCLNT_STREAMFLAGS_LOOPBACK被设置，IAudioClient::Initialize会尝试
    在rending设备开辟一块capture buffer。
    AUDCLNT_STREAMFLAGS_LOOPBACK只对rending设备有效，
    Initialize仅在AUDCLNT_SHAREMODE_SHARED时才可以使用, 否则Initialize会失败。
    Initialize成功后，可以用IAudioClient::GetService可获取该rending设备的IAudioCaptureClient接口。
    */
    /*
    * 设备初始化：
    * 1、为音频流指定共享模式还是独占模式
    * 2、控制 流创建的标志
    * 3、缓冲区大小
    * 4、设备周期
    * 5、指向音频数据的格式的指针
    * 6、指向会话GUID的指针
    */
    printf("\nAUDCLNT_STREAMFLAGS_LOOPBACK...\n\n");
    hr = pAudioClient->Initialize(
        AUDCLNT_SHAREMODE_SHARED,
        AUDCLNT_STREAMFLAGS_LOOPBACK,
        hnsRequestedDuration,
        0,
        pwfx,
        NULL);

    if (FAILED(hr)) {
        CoTaskMemFree(pwfx);
        SAFE_RELEASE(pEnumerator)
            SAFE_RELEASE(pDevice)
            SAFE_RELEASE(pAudioClient)
            SAFE_RELEASE(pCaptureClient)
            SAFE_RELEASE(pCollection)


            CoUninitialize();

        if (pbyCaptureBuffer)
        {
            delete[] pbyCaptureBuffer;
            pbyCaptureBuffer = NULL;
        }

        getchar();

        return 0;
    }

        /*
            https://msdn.microsoft.com/en-us/library/windows/desktop/dd370874(v=vs.85).aspx

            GetStreamLatency 方法检索当前流的最大延迟，并且可以在流初始化后随时调用。
            This method requires prior initialization of the IAudioClient interface.
            All calls to this method will fail with the error AUDCLNT_E_NOT_INITIALIZED until
            the client initializes the audio stream by successfully calling the IAudioClient::Initialize method.
            This method retrieves the maximum latency for the current stream.
            The value will not change for the lifetime of the IAudioClient object.
            Rendering clients can use this latency value to compute the minimum amount of data
            that they can write during any single processing pass.
            To write less than this minimum is to risk introducing glitches into the audio stream.
            For more information, see IAudioRenderClient::GetBuffer.

            1. 该函数返回当前流的最大延时 在IAudioClient对象的生命周期内 不会发生变化
            2. Rendering客户端可以用这个延时值，来计算每次处理pass可以写的最小数据量。
            注: 使用前须先调用IAudioClient::Initialize
        */

        REFERENCE_TIME hnsStreamLatency;
    hr = pAudioClient->GetStreamLatency(&hnsStreamLatency);
    if (FAILED(hr)) {
        CoTaskMemFree(pwfx);
        SAFE_RELEASE(pEnumerator)
            SAFE_RELEASE(pDevice)
            SAFE_RELEASE(pAudioClient)
            SAFE_RELEASE(pCaptureClient)
            SAFE_RELEASE(pCollection)


            CoUninitialize();

        if (pbyCaptureBuffer)
        {
            delete[] pbyCaptureBuffer;
            pbyCaptureBuffer = NULL;
        }

        getchar();

        return 0;
    }

        cout << "GetStreamLatency     : " << hnsStreamLatency
        << " REFERENCE_TIME time units. 即(" << hnsStreamLatency / 10000 << "ms)" << endl;

    /*
        phnsDefaultDevicePeriod [out]
        Pointer to a REFERENCE_TIME variable into which the method writes a time value
        specifying the default interval between periodic processing passes by the audio engine.
        The time is expressed in 100-nanosecond units.

        phnsMinimumDevicePeriod [out]
        Pointer to a REFERENCE_TIME variable into which the method writes a time value
        specifying the minimum interval between periodic processing passes by the audio endpoint device.
        The time is expressed in 100-nanosecond units.
    */

    REFERENCE_TIME hnsDefaultDevicePeriod;
    REFERENCE_TIME hnsMinimumDevicePeriod;
    hr = pAudioClient->GetDevicePeriod(&hnsDefaultDevicePeriod, &hnsMinimumDevicePeriod);
    if (FAILED(hr)) {
        CoTaskMemFree(pwfx);
        SAFE_RELEASE(pEnumerator)
            SAFE_RELEASE(pDevice)
            SAFE_RELEASE(pAudioClient)
            SAFE_RELEASE(pCaptureClient)
            SAFE_RELEASE(pCollection)


            CoUninitialize();

        if (pbyCaptureBuffer)
        {
            delete[] pbyCaptureBuffer;
            pbyCaptureBuffer = NULL;
        }

        getchar();

        return 0;
    }

        cout << "GetDevicePeriod  ...\n"
        << "hnsDefaultDevicePeriod : " << hnsDefaultDevicePeriod
        << " REFERENCE_TIME time units. 即(" << hnsDefaultDevicePeriod / 10000 << "ms)" << endl
        << "hnsMinimumDevicePeriod : " << hnsMinimumDevicePeriod
        << " REFERENCE_TIME time units. 即(" << hnsMinimumDevicePeriod / 10000 << "ms)" << endl;

    // 获取已分配的缓冲区的大小
    // 这个buffersize，指的是缓冲区最多可以存放多少帧的数据量
    /*
        https://msdn.microsoft.com/en-us/library/dd370866(v=vs.85).aspx

        pNumBufferFrames [out]
        Pointer to a UINT32 variable into which the method writes the number of audio frames
        that the buffer can hold.
        The IAudioClient::Initialize method allocates the buffer.
        The client specifies the buffer length in the hnsBufferDuration parameter value
        that it passes to the Initialize method. For rendering clients,
        the buffer length determines the maximum amount of rendering data
        that the application can write to the endpoint buffer during a single processing pass.
        For capture clients, the buffer length determines the maximum amount of capture data
        that the audio engine can read from the endpoint buffer during a single processing pass.

        The client should always call GetBufferSize after calling Initialize
        to determine the actual size of the allocated buffer,
        which might differ from the requested size.
        Rendering clients can use this value to calculate the largest rendering buffer size
        that can be requested from IAudioRenderClient::GetBuffer during each processing pass.
    */
    hr = pAudioClient->GetBufferSize(&bufferFrameCount);
    if (FAILED(hr)) {
        CoTaskMemFree(pwfx);
        SAFE_RELEASE(pEnumerator)
            SAFE_RELEASE(pDevice)
            SAFE_RELEASE(pAudioClient)
            SAFE_RELEASE(pCaptureClient)
            SAFE_RELEASE(pCollection)


            CoUninitialize();

        if (pbyCaptureBuffer)
        {
            delete[] pbyCaptureBuffer;
            pbyCaptureBuffer = NULL;
        }

        getchar();

        return 0;
    }
        cout << endl << "GetBufferSize        : " << bufferFrameCount << endl;

    // SetEventHandle
    //
    /*HANDLE hAudioSamplesReadyEvent = CreateEventEx(NULL, NULL, 0, EVENT_MODIFY_STATE | SYNCHRONIZE);
    if (hAudioSamplesReadyEvent == NULL)
    {
        printf("Unable to create samples ready event: %d.\n", GetLastError());
        goto Exit;
    }*/

    /*
        当音频的buffer就绪 可被client处理时， 会发出系统信号
        SetEventHandle用于设置处理该信号的event的handle
        During stream initialization, the client can, as an option, enable event-driven buffering.
        To do so, the client calls the IAudioClient::Initialize method
        with the AUDCLNT_STREAMFLAGS_EVENTCALLBACK flag set.
        After enabling event-driven buffering,
        and before calling the IAudioClient::Start method to start the stream,
        the client must call SetEventHandle to register the event handle
        that the system will signal each time a buffer becomes ready to be processed by the client.
        使用SetEventHandle， 需要在IAudioClient::Initialize设置AUDCLNT_STREAMFLAGS_EVENTCALLBACK。
        SetEventHandle应该在调用IAudioClient::Start之前调用。
    */
    /*hr = pAudioClient->SetEventHandle(hAudioSamplesReadyEvent);
    if (FAILED(hr))
    {
        printf("Unable to set ready event: %x.\n", hr);
        return false;
    }*/
    //

    // 创建采集管理接口
    hr = pAudioClient->GetService(IID_IAudioCaptureClient, (void**)&pCaptureClient);
    if (FAILED(hr)) {
        CoTaskMemFree(pwfx);
        SAFE_RELEASE(pEnumerator)
            SAFE_RELEASE(pDevice)
            SAFE_RELEASE(pAudioClient)
            SAFE_RELEASE(pCaptureClient)
            SAFE_RELEASE(pCollection)


            CoUninitialize();

        if (pbyCaptureBuffer)
        {
            delete[] pbyCaptureBuffer;
            pbyCaptureBuffer = NULL;
        }

        getchar();

        return 0;
    }

        //
        hnsActualDuration = (double)REFTIMES_PER_SEC * bufferFrameCount / pwfx->nSamplesPerSec;

    hr = pAudioClient->Start();  // Start recording.
    if (FAILED(hr)) {
        CoTaskMemFree(pwfx);
        SAFE_RELEASE(pEnumerator)
            SAFE_RELEASE(pDevice)
            SAFE_RELEASE(pAudioClient)
            SAFE_RELEASE(pCaptureClient)
            SAFE_RELEASE(pCollection)


            CoUninitialize();

        if (pbyCaptureBuffer)
        {
            delete[] pbyCaptureBuffer;
            pbyCaptureBuffer = NULL;
        }

        getchar();

        return 0;
    }

        printf("\nAudio Capture begin...\n\n");

    int  nCnt = 0;

    size_t nCaptureBufferSize = 8 * 1024 * 1024;
    size_t nCurrentCaptureIndex = 0;

    pbyCaptureBuffer = new (std::nothrow) BYTE[nCaptureBufferSize];

    bool stillPlaying = true;

    // 每个循环填充大约一半的共享缓冲区
    while (stillPlaying)
    {
        // 让程序暂停500毫秒，以便将数据填充到缓冲区
        cout << "hnsActualDuration / REFTIMES_PER_MILLISEC / 2 = "
            << hnsActualDuration / REFTIMES_PER_MILLISEC / 2 << endl;
        Sleep(hnsActualDuration / REFTIMES_PER_MILLISEC / 2);
        hr = pCaptureClient->GetNextPacketSize(&packetLength);
        EXIT_ON_ERROR(hr)

            printf("%06d # _AudioSamplesReadyEvent packetLength:%06u \n", nCnt, packetLength);

        while (packetLength != 0)
        {
            // 从共享缓冲区中获取可用的数据，锁定缓冲区，获取数据
            hr = pCaptureClient->GetBuffer(&pData, &numFramesAvailable, &flags, NULL, NULL);
            EXIT_ON_ERROR(hr)
                nCnt++;
            // test flags
            //
            if (flags & AUDCLNT_BUFFERFLAGS_SILENT)
            {
                printf("AUDCLNT_BUFFERFLAGS_SILENT \n");
            }

            if (flags & AUDCLNT_BUFFERFLAGS_DATA_DISCONTINUITY)
            {
                printf("%06d # AUDCLNT_BUFFERFLAGS_DATA_DISCONTINUITY \n", nCnt);
            }
            //

            UINT32 framesToCopy = min(numFramesAvailable,
                static_cast<UINT32>((nCaptureBufferSize - nCurrentCaptureIndex) / nFrameSize));
            if (framesToCopy != 0)
            {
                //
                //  The flags on capture tell us information about the data.
                //
                //  We only really care about the silent flag since we want to put frames of silence into the buffer
                //  when we receive silence.  We rely on the fact that a logical bit 0 is silence for both float and int formats.
                //
                if (flags & AUDCLNT_BUFFERFLAGS_SILENT)
                {
                    // 用 0 来填充一块内存区域：将捕获缓冲区中的 0 填充到输出缓存区中
                    ZeroMemory(&pbyCaptureBuffer[nCurrentCaptureIndex], framesToCopy * nFrameSize);
                }
                else
                {
                    // 从音频音频复制数据到输出缓冲区
                    CopyMemory(&pbyCaptureBuffer[nCurrentCaptureIndex], pData, framesToCopy * nFrameSize);
                }
                //  Bump the capture buffer pointer.
                nCurrentCaptureIndex += framesToCopy * nFrameSize;
            }

            hr = pCaptureClient->ReleaseBuffer(numFramesAvailable);
            EXIT_ON_ERROR(hr)

                hr = pCaptureClient->GetNextPacketSize(&packetLength);
            EXIT_ON_ERROR(hr)

                /*
                    GetCurrentPadding 方法获取在端点设备中排队的数据量（以 100 纳秒为单位）。
                    即获取缓冲区中已捕获的但未读的数据总量，
                    在任何时刻，数据包的大小始终小于或等于缓冲区中的数据总量
                    The padding value的单位是audio frame
                    一个audio frame的大小等于 通道数 * 每个通道的sample大小
                */
                UINT32 ui32NumPaddingFrames;
            hr = pAudioClient->GetCurrentPadding(&ui32NumPaddingFrames);
            EXIT_ON_ERROR(hr)
                if (0 != ui32NumPaddingFrames)
                {
                    printf("GetCurrentPadding : %6u\n", ui32NumPaddingFrames);
                }

            // 采集一定数目个buffer后退出
            if (nCnt == 64)
            {
                stillPlaying = false;
                break;
            }
        }
    }

    // 将捕获的数据保存到WAV文件
    SaveWaveData(pbyCaptureBuffer, nCurrentCaptureIndex, pwfx, "C:\\Users\\mm133\\Desktop\\Code\\wenet-main\\runtime\\core\\build\\SOSForWindows\\Debug");
    this->bufferUtil->length = nCurrentCaptureIndex * 16000 / pwfx->nSamplesPerSec;
    this->bufferUtil->Buffer = new BYTE[this->bufferUtil->length];
    pcm_downsample(pbyCaptureBuffer, nCurrentCaptureIndex, pwfx, this->bufferUtil->Buffer,&this->bufferUtil->length);

    printf("\nAudio Capture Done.\n");

    hr = pAudioClient->Stop();
    EXIT_ON_ERROR(hr)

Exit:
    CoTaskMemFree(pwfx);
    SAFE_RELEASE(pEnumerator)
        SAFE_RELEASE(pDevice)
        SAFE_RELEASE(pAudioClient)
        SAFE_RELEASE(pCaptureClient)
        SAFE_RELEASE(pCollection)


        CoUninitialize();

    if (pbyCaptureBuffer)
    {
        delete[] pbyCaptureBuffer;
        pbyCaptureBuffer = NULL;
    }

    return 0;
}
