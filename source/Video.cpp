#include "Video.h"

/*
Change Log:
- created by Kenny, Yitong, Melanie, and Cole for the final
*/

void VideoRecorder::stopRecording() {
    debugAssert(m_video);

    // Save the movie
    m_video->commit();
    String oldFilename = m_video->filename();
    String newFilename = oldFilename;
    m_video.reset();
}

//Assert: numFrames > 0
void VideoRecorder::recordFrame(const shared_ptr<Texture> frame) {
    debugAssert(m_video);

    m_video->append(frame);
    --numFrames;

    if (numFrames <= 0) {

        //append the last frame a bunch of times.
        //video seems to be cutting out the last 2-3 seconds or so.
        //this is a hack
        for(int i = 0; i < 120; ++i) {
            m_video->append(frame);
        }

        stopRecording();
    }
}

void VideoRecorder::startRecording(Point2 dim, String filenamePrefix, float numSeconds) {
    dimensions = dim;
    const int fps = 30;
    numFrames = numSeconds * fps;
    
    //video format
    VideoOutput::Settings setting = VideoOutput::Settings::MPEG4(dimensions.x, dimensions.y);
    debugAssert(isNull(m_video));
    setting.fps = fps;

    const String filename = filenamePrefix + "." + setting.extension;
    m_video = VideoOutput::create(filename, setting);
}

