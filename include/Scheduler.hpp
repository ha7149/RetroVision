#ifndef SCHEDULER_HPP
#define SCHEDULER_HPP

#include <chrono>

class Scheduler {
private:
    std::chrono::steady_clock::time_point broadcastStartTime;

public:
    // Functional Statement: Initializes the master broadcast timeline
    Scheduler();

    // Functional Statement: Calculates total elapsed seconds since engine start (FR-001)
    double GetElapsedBroadcastTime() const;

    // Functional Statement: Calculates the exact seek position for continuous infinite looping (FR-008)
    double CalculateSeekTimestamp(double videoDurationInSeconds) const;

    // Optional: Resets the timeline (Useful for debugging)
    void ResetTimeline();
};

#endif // SCHEDULER_HPP