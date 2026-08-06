#include "Decoder.hpp"
#include <iostream>

Decoder::Decoder() = default;

Decoder::~Decoder() {
    CleanupPreviousContext();
}

void Decoder::CleanupPreviousContext() {
    if (codecCtx) {
        avcodec_free_context(&codecCtx);
        codecCtx = nullptr;
    }
    if (fmtCtx) {
        avformat_close_input(&fmtCtx);
        fmtCtx = nullptr;
    }
    videoStreamIndex = -1;
}

bool Decoder::OpenMedia(const std::string& filepath) {
    CleanupPreviousContext();

    if (avformat_open_input(&fmtCtx, filepath.c_str(), nullptr, nullptr) < 0) {
        std::cerr << "[Decoder Error] Failed to open media container: " << filepath << std::endl;
        return false;
    }

    if (avformat_find_stream_info(fmtCtx, nullptr) < 0) {
        std::cerr << "[Decoder Error] Failed to retrieve stream information from: " << filepath << std::endl;
        CleanupPreviousContext();
        return false;
    }

    videoStreamIndex = -1;
    for (unsigned int i = 0; i < fmtCtx->nb_streams; ++i) {
        if (fmtCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoStreamIndex = static_cast<int>(i);
            break;
        }
    }

    if (videoStreamIndex == -1) {
        std::cerr << "[Decoder Error] No valid video stream found in: " << filepath << std::endl;
        CleanupPreviousContext();
        return false;
    }

    const AVCodec* codec = avcodec_find_decoder(fmtCtx->streams[videoStreamIndex]->codecpar->codec_id);
    if (!codec) {
        std::cerr << "[Decoder Error] Unsupported video codec." << std::endl;
        CleanupPreviousContext();
        return false;
    }

    codecCtx = avcodec_alloc_context3(codec);
    if (!codecCtx) {
        CleanupPreviousContext();
        return false;
    }

    if (avcodec_parameters_to_context(codecCtx, fmtCtx->streams[videoStreamIndex]->codecpar) < 0) {
        CleanupPreviousContext();
        return false;
    }

    if (avcodec_open2(codecCtx, codec, nullptr) < 0) {
        std::cerr << "[Decoder Error] Failed to initialize codec context." << std::endl;
        CleanupPreviousContext();
        return false;
    }

    std::cout << "[Decoder] Successfully loaded: " << filepath << std::endl;
    return true;
}

bool Decoder::SeekToTime(double timestamp) {
    if (!fmtCtx) return false;

    int64_t seekTarget = static_cast<int64_t>(timestamp * AV_TIME_BASE);
    // AVSEEK_FLAG_BACKWARD ensures we land on a keyframe
    if (av_seek_frame(fmtCtx, -1, seekTarget, AVSEEK_FLAG_BACKWARD) >= 0) {
        if (codecCtx) {
            avcodec_flush_buffers(codecCtx); // Clear internal decoder buffers post-seek
        }
        return true;
    }

    return false;
}

// Added: FetchFrame implementation
bool Decoder::FetchFrame(AVFrame* frame) {
    if (!fmtCtx || !codecCtx || videoStreamIndex == -1) return false;

    AVPacket* packet = av_packet_alloc();
    if (!packet) return false;

    bool frameDecoded = false;

    while (av_read_frame(fmtCtx, packet) >= 0) {
        if (packet->stream_index == videoStreamIndex) {
            if (avcodec_send_packet(codecCtx, packet) == 0) {
                if (avcodec_receive_frame(codecCtx, frame) == 0) {
                    frameDecoded = true;
                    av_packet_free(&packet);
                    break;
                }
            }
        }
        av_packet_unref(packet);
    }

    av_packet_free(&packet);
    return frameDecoded;
}