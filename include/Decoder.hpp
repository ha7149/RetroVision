#ifndef DECODER_HPP
#define DECODER_HPP

#include <string>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}

class Decoder {
private:
    AVFormatContext* fmtCtx = nullptr;
    AVCodecContext* codecCtx = nullptr;
    int videoStreamIndex = -1;

    // Decoupled memory cleanup (FR-014)
    void CleanupPreviousContext();

public:
    Decoder();
    ~Decoder();

    // Open video resource and initialize decoders
    bool OpenMedia(const std::string& filepath);

    // Execute fast-seek accurate to keyframes (FR-009)
    bool SeekToTime(double timestamp);

    // Added: Fetch decoded video frame
    bool FetchFrame(AVFrame* frame);

    // Getters
    bool IsOpen() const { return fmtCtx != nullptr; }
    int GetVideoStreamIndex() const { return videoStreamIndex; }
};

#endif // DECODER_HPP