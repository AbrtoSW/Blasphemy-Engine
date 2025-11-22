#include "engine/vk_frame_manager.h"

FrameManager::FrameManager(uint32_t frameCount) : frames(frameCount), currentIndex(0) {}

FrameData& FrameManager::currentFrame() {
	return frames[currentIndex];
}

FrameData& FrameManager::getFrame(uint32_t index) {
	return frames[index];
}

uint32_t  FrameManager::getCurrentIndex() const {
	return currentIndex;
}


void FrameManager::advance() {
	currentIndex = (currentIndex + 1) % frames.size();
}


