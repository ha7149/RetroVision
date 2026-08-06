#include "Scheduler.hpp"
#include <cmath>

Scheduler::Scheduler() {
    ResetTimeline();
}

void Scheduler::ResetTimeline() {
    // Stamps the exact OS time the application booted
    broadcastStartTime = std::chrono::steady_clock::now();
}

double Scheduler::GetElapsedBroadcastTime() const {
    auto currentTime = std::chrono::steady_clock::now();
    std::chrono::duration<double> elapsed = currentTime - broadcastStartTime;
    return elapsed.count(); // Returns exact fractional seconds
}

double Scheduler::CalculateSeekTimestamp(double videoDurationInSeconds) const {
    // Error Handling: Prevent divide-by-zero if media is invalid
    if (videoDurationInSeconds <= 0.0) return 0.0;
    
    double elapsed = GetElapsedBroadcastTime();
    
    // Core Engine Logic (FR-008): 
    // std::fmod returns the floating-point remainder. 
    // Example: If elapsed is 500s and video is 120s, it returns 20s.
    return std::fmod(elapsed, videoDurationInSeconds);
}