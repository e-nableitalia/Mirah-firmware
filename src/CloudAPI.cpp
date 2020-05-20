#include <CloudAPI.hpp>
#include <Telemetry.hpp>
#include <sEMG.hpp>
#include <Motor.hpp>

#define SEMG_BUFFER_DEPTH       1000 // 1 second
#define TELEMETRY_SEMG_PAYLOAD  1000 // 1000 bytes -> 1 pkt every 500ms

int clock_rate = SAMPLE_FREQ_1000HZ;

CloudAPI cloudapi;

int _enableConsole(String args)  { return cloudapi.enableConsole(args); };
int _enableStreaming(String args)  { return cloudapi.enableStreaming(args); };
int _setEmgBaseline(String args) { return cloudapi.setEmgBaseline(args); };
int _setEmgOffset(String args) { return cloudapi.setEmgOffset(args); };
int _setRemotePort(String args) { return cloudapi.setRemotePort(args); };
int _setRemoteIP(String args) { return cloudapi.setRemoteIP(args); };

int _setEmgChannels(String args) { return cloudapi.setEmgChannels(args); };
int _setEmgCurrentChannel(String args) { return cloudapi.setEmgCurrentChannel(args); };
int _setEmgChannelSampleRate(String args) { return cloudapi.setEmgChannelSampleRate(args); };
int _setEmgNotchFilter(String args) { return cloudapi.setEmgNotchFilter(args); };
int _setEmgLowpassFilter(String args) { return cloudapi.setEmgLowpassFilter(args); };
int _setEmgHighpassFilter(String args) { return cloudapi.setEmgHighpassFilter(args); };

void _timer_handler() { cloudapi.timer_handler(); };

void notify(String event) {

    C_DEBUG("notify %s", event.c_str());

    Particle.publish(event,PRIVATE);
}

bool isTrue(String value) {

    C_DEBUG("isTrue(%s)",value.c_str());

    if (value.toLowerCase() == "true") {
        C_DEBUG("returning true");
        return true;
    } else {
        C_DEBUG("returning false");
        return false;
    }
}

CloudAPI::CloudAPI() {
    ip = "";

    enabled = false;
    remoteIP = "";
    remotePort = 0;
    status = "";

    semg_channels = semg_sample_rate = 0;
    semg_current_channel = -1;
}

void CloudAPI::init(bool debug) {
    telemetry.init(true);

    C_DEBUG("initializing");

    buffer.size(SEMG_BUFFER_DEPTH);

    semg_current_channel = -1;

    semg_channels = 0;

    telemetry.attach(SEMG_CHANNEL0,this);

    telemetry.enable(SEMG_CHANNEL0);

    IPAddress myIP = WiFi.localIP();
   
    ip = String::format("%d.%d.%d.%d", (int)myIP[0],(int) myIP[1], (int)myIP[2],(int) myIP[3]);

    String message = String::format("Board IP[%s]",ip.c_str());

    C_DEBUG(message);
    notify(message);

    status = "idle";
    enabled = false;
    remoteIP = "";
    remotePort = 0;

    PQ12Motor.init(false,true);
    
    C_DEBUG("Publishing variables");

    Particle.variable("streaming.localIP", ip);
    Particle.variable("streaming.status", status);
    Particle.variable("emg.channel", semg_current_channel);
    Particle.variable("emg.numchannels", semg_channels);
    Particle.variable("emg.sample_rate", semg_sample_rate);

    C_DEBUG("Publishing functions");    
    
    Particle.function("console.enable", _enableConsole);

    Particle.function("streaming.remoteIP", _setRemoteIP);
    Particle.function("streaming.remotePort", _setRemotePort);

    Particle.function("streaming.enable", _enableStreaming);

    Particle.function("emg.setnumchannels", _setEmgChannels);
    Particle.function("emg.setchannel", _setEmgCurrentChannel);
    Particle.function("emg.samplerate", _setEmgChannelSampleRate);
    Particle.function("emg.offset", _setEmgOffset);
    Particle.function("emg.baseline", _setEmgBaseline);
    Particle.function("emg.notchfilter", _setEmgNotchFilter);
    Particle.function("emg.lowpassfilter", _setEmgLowpassFilter);
    Particle.function("emg.highpassfilter", _setEmgHighpassFilter);

     // Time interval for processing the input signal.
    C_DEBUG("Configuring timer, clock rate[%d Hz]", clock_rate);

    if (clock_rate >= SAMPLE_FREQ_500HZ) {
        uint16_t intPeriod = (clock_rate == SAMPLE_FREQ_500HZ ? 2000 : 1000);

        C_DEBUG("Starting Timer, period[%d us]", intPeriod);

        timer.begin(_timer_handler, intPeriod, uSec);
    } else {
        uint16_t intPeriod = 2000 / clock_rate;

        C_DEBUG("Starting Timer, period[%d half ms]", intPeriod);

        timer.begin(_timer_handler, intPeriod, hmSec);
    }

    notify("board init complete");

    C_DEBUG("Init complete");
    
    telemetry.console(debug);
}

int CloudAPI::enableConsole(String args) {
    C_DEBUG("EnableConsole(%s)", args.c_str());
    if (isTrue(args)) {
        telemetry.console(true);
    } else {
        telemetry.console(false);
    }
    return 0;
}

int CloudAPI::enableStreaming(String args) {
    C_DEBUG("EnableStreaming(%s)", args.c_str());
    if (isTrue(args)) {
        C_DEBUG("EnableStreaming, remoteIP[%s], remotePort[%d]", remoteIP.c_str(), remotePort);
        if (telemetry.streaming_enable(remoteIP, remotePort)) {
            C_DEBUG("Streaming activated");
            status="active";
            enabled = true;
            notify("streaming enabled");
        } else {
            C_DEBUG("Streaming not activated");
            status="idle";
            enabled = false;
            notify("streaming not enabled");
        }
    } else if (enabled) {
        C_DEBUG("Disabling streaming");
        telemetry.streaming_disable();
        status="idle";
        enabled = false;
        notify("streaming disabled");
    }
    return 0;
}

int  CloudAPI::setEmgBaseline(String args) {
    C_DEBUG("setEmgBaseline(%d:%s)", semg_current_channel, args.c_str());
    
    if ((cloudapi.semg_current_channel>=0) && (semg_current_channel<MAX_EMG_CHANNELS)) {
        semg[semg_current_channel].enableEnvelope(args.toInt());
    
        String message = String::format("EMG[%d] Activated mode baseline[%s]",semg_current_channel,args.c_str());

        notify(message);
    } else {
        String message = String::format("EMG[%d] invalid channel",semg_current_channel);
        notify(message);
    }

    return 0;
}

int  CloudAPI::setEmgChannels(String args) {
    C_DEBUG("setEmgChannels(%s)", args.c_str());

    int channel = args.toInt();

    timer.interrupt_SIT(INT_DISABLE);
    if ((channel >=0) && (channel <= MAX_EMG_CHANNELS)) {
        semg_channels = channel;
    } else
        semg_channels = 0;

   // reset channels
    for (int i = 0; i < MAX_EMG_CHANNELS; i++) {
        
        if (i < semg_channels) {
            C_DEBUG("Configuring EMG Channel[%d]", i);
            semg[i].init(semg_sample_rate,clock_rate,A0 + i,true);

            semg[i].setBuffer(&buffer);
        } else {
            C_DEBUG("Disabling EMG Channel[%d]", i);
            semg[i].disable();
        }
    }
    timer.interrupt_SIT(INT_ENABLE);

    return 0;
}

int  CloudAPI::setEmgCurrentChannel(String args) {
    C_DEBUG("setEmgCurrentChannel(%s)", args.c_str());

    int channel = args.toInt();

    if ((channel >=0) && (channel < MAX_EMG_CHANNELS)) {
        semg_current_channel = channel;
    } else
        semg_current_channel = -1;

    return 0;
}

int  CloudAPI::setEmgChannelSampleRate(String args) {
    C_DEBUG("setEmgChannelSampleRate(%s)", args.c_str());

    int srate = args.toInt();

    if ((srate >=0) && (srate <= clock_rate)) {
        semg_sample_rate = srate;
    } else
        semg_sample_rate = 0;

    return 0;
}



int  CloudAPI::setEmgOffset(String args) {
    C_DEBUG("setEmgOffset(%d:%s)", semg_current_channel, args.c_str());
    
    if ((semg_current_channel>=0) && (semg_current_channel<MAX_EMG_CHANNELS)) {
        semg[semg_current_channel].setOffset(args.toInt());
    
        String message = String::format("EMG[%d] Activated mode offset[%s]",semg_current_channel,args.c_str());

        notify(message);
    } else {
        String message = String::format("EMG[%d] invalid channel",semg_current_channel);
        notify(message);
    }

    return 0;
}

int  CloudAPI::setRemotePort(String args) {
    C_DEBUG("setRemotePort(%s)", args.c_str());
    remotePort = args.toInt();

    notify("set remote port " + args);

    return 0;
}

int  CloudAPI::setRemoteIP(String args) {
    C_DEBUG("setRemoteIP(%s)", args.c_str());
    remoteIP = args;

    notify("set remote IP " + args);

    return 0;
}

int CloudAPI::setEmgNotchFilter(String args) {
   C_DEBUG("setEmgNotchFilter(%d:%s)", semg_current_channel, args.c_str());
    
    if ((semg_current_channel>=0) && (semg_current_channel<MAX_EMG_CHANNELS)) {
        
        bool filter_enabled = false;
        if (isTrue(args)) {
            filter_enabled = semg[semg_current_channel].enableFilter(NOTCH_FILTER,true);
        } else
            filter_enabled = semg[semg_current_channel].enableFilter(NOTCH_FILTER,false);
    
        String message = String::format("EMG[%d] set notch filter[%s], filters enabled[%s]",semg_current_channel,args.c_str(),BOOL_STR(filter_enabled));

        notify(message);
    } else {
        String message = String::format("EMG[%d] invalid channel",semg_current_channel);
        notify(message);
    }

    return 0;
}

int CloudAPI::setEmgLowpassFilter(String args) {
    C_DEBUG("setEmgLowpassFilter(%d:%s)", semg_current_channel, args.c_str());
    
    if ((semg_current_channel>=0) && (semg_current_channel<MAX_EMG_CHANNELS)) {
        
        bool filter_enabled = false;
        if (isTrue(args)) {
            filter_enabled = semg[semg_current_channel].enableFilter(LOWPASS_FILTER,true);
        } else
            filter_enabled = semg[semg_current_channel].enableFilter(LOWPASS_FILTER,false);
    
        String message = String::format("EMG[%d] set lowpass filter[%s], filters enabled[%s]",semg_current_channel,args.c_str(),BOOL_STR(filter_enabled));

        notify(message);
    } else {
        String message = String::format("EMG[%d] invalid channel",semg_current_channel);
        notify(message);
    }

    return 0;
}

int CloudAPI::setEmgHighpassFilter(String args) {
    C_DEBUG("setEmgHighpassFilter(%d:%s)", semg_current_channel, args.c_str());
    
    if ((semg_current_channel>=0) && (semg_current_channel<MAX_EMG_CHANNELS)) {
        
        bool filter_enabled = false;
        if (isTrue(args)) {
            filter_enabled = semg[semg_current_channel].enableFilter(HIGHPASS_FILTER,true);
        } else
            filter_enabled = semg[semg_current_channel].enableFilter(HIGHPASS_FILTER,false);
    
        String message = String::format("EMG[%d] set highpass filter[%s], filters enabled[%s]",semg_current_channel,args.c_str(),BOOL_STR(filter_enabled));

        notify(message);
    } else {
        String message = String::format("EMG[%d] invalid channel",semg_current_channel);
        notify(message);
    }
    return 0;
}

void CloudAPI::poll() {

    PQ12Motor.poll();
   
    // return if no streaming enabled
    if (!enabled) return;
    C_DEBUG("Cloudapi.poll");
    timer.interrupt_SIT(INT_DISABLE);
    C_DEBUG("Cloudapi.poll 2");

    telemetry.poll();
    timer.interrupt_SIT(INT_ENABLE);
    telemetry.send();
}

void CloudAPI::timer_handler() {

    for (int i = 0; i < MAX_EMG_CHANNELS; i++)
        if (semg[i].enabled())
            semg[i].poll();
}

int CloudAPI::consume(uint8_t *b, int size, bool fill) {
    int result = 0;

    size >>= 1;

    C_DEBUG("consuming[%d] bytes, avail samples[%d]", size * 2, buffer.avail());

    if ((size <= buffer.avail()) && (fill)) {
        C_DEBUG("consuming[%d] samples", size);
        result = buffer.consume(b,size);
    } else if (!fill) {
        // calculate new size
        size = buffer.avail()<<1;
        C_DEBUG("fill[false], consuming[%d] bytes", size);
        result = buffer.consume(b,size);
    } 
    
    C_DEBUG("samples consumed[%d]", result);

    return result*2;
}
