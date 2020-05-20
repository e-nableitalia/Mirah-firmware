//
// CloudAPI: systemi init & cloud API for system control
//
// Author: A.Navatta

#ifndef CLOUD_API_H

#define CLOUD_API_H

#include <application.h>
#include <SparkIntervalTimer.h>
#include <sEMG.hpp>
#include <BufferProducer.hpp>

#define DEFAULT_SAMPLE_FREQUENCY    1000
#define DEFAULT_STREAMING_SAMPLES   500

#define MAX_EMG_CHANNELS            2

class CloudAPI : public BufferProducer {
    public:
        CloudAPI();
        void init(bool debug = true);
        void poll();

        int enableConsole(String args);
        int enableStreaming(String args);
        int setEmgBaseline(String args);
        int setEmgOffset(String args);
        int setRemotePort(String args);
        int setRemoteIP(String args);

        int setEmgChannels(String args);
        int setEmgCurrentChannel(String args);
        int setEmgChannelSampleRate(String args);
        int setEmgNotchFilter(String args);
        int setEmgLowpassFilter(String args);
        int setEmgHighpassFilter(String args);

        void timer_handler();

    private:
        // consume size samples in buffer
        // returns number of consumed samples
        // zero if internal buffer doesn't contains enough data
        // note: each sample is a short --> consumes 2 uint8_t in the buffer
        virtual int consume(uint8_t *buffer, int size, bool fill = true);

        // local ip
        String ip;
        // telemetry
        // remote telemetry enabled
        bool enabled;
        // remote ip
        String remoteIP;
        // remote port
        int remotePort;
        // telemetry status
        String status;
        // high frequency timer
        IntervalTimer timer;
        // EMG
        int semg_current_channel;
        int semg_channels;
        int semg_sample_rate;
        sEMG semg[MAX_EMG_CHANNELS];
        // buffer
        CircularBuffer buffer;        
};

extern CloudAPI cloudapi;

#endif