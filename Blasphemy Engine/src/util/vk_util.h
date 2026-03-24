#pragma once

#include <vector>

#include "volk/volk.h"
#include <vma/vk_mem_alloc.h>

#include "types/vk_types.h"

#define VK_CHECK(x)                                                     \
    do {                                                                \
        VkResult err = x;                                               \
        if (err) {                                                      \
            fprintf(stderr, "Vulkan error: %d at %s:%d\n",              \
                    err, __FILE__, __LINE__);                           \
            abort();                                                    \
        }                                                               \
    } while (0)




struct DeletionQueue {

public:
	void flushFrameResources(VmaAllocator& vmaAllocator);

	void flushMainResources(VkDevice device, VmaAllocator& vmaAllocator);
	void flushResize(VkDevice device, VmaAllocator& vmaAllocator);

	inline void pushFrameBuffer(const AllocatedBuffer& buffer) { vmaFrameAllocatedBuffer.push_back(buffer); }
	inline void pushBuffer(const AllocatedBuffer& buffer) { vmaAllocatedBuffer.push_back(buffer); }

	inline void pushImage(const AllocatedImage& image) { allocatedImages.push_back(image); }
	inline void pushOffscreenImage(const AllocatedImage& image) { offscreenImages.push_back(image); }

	inline void pushRenderPass(const VkRenderPass& renderPass) { renderPasses.push_back(renderPass); }

private:
	std::vector<AllocatedBuffer> vmaFrameAllocatedBuffer;
	std::vector<AllocatedBuffer> vmaAllocatedBuffer;
	std::vector<AllocatedImage> offscreenImages;
	std::vector<AllocatedImage> allocatedImages;
	std::vector<VkRenderPass> renderPasses;
};

