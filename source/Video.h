#pragma once
#include <G3D/G3DAll.h>
class VideoRecorder{
private:
    shared_ptr<VideoOutput> m_video;

public:
    int numFrames = 0;
    Point2 dimensions;

    /** Adds the passed texture to the output video. */
    void recordFrame(const shared_ptr<Texture> frame);

    /** Finishes creating the video file. */
    void stopRecording();

    /** Creates a video file. */
    void startRecording(Point2 dimensions, String filenamePrefix, float numSeconds);
};