#pragma once

#include <vector>
#include <deque>
#include <functional>

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

	std::vector<VkPipeline> pipelines;

	std::deque<std::function<void()>> deletors;

	void push_deletion_lambda(std::function<void()>&& function);

	void flush_deletion_lambda();

	void flushFrameResources(VmaAllocator& vmaAllocator);

	void flushMainResources(VkDevice device, VmaAllocator& vmaAllocator);

	void resizeFlush(VkDevice device, VmaAllocator& vmaAllocator);

	template <typename T>
	inline void push_sampler(const T& s) {
		samplers.push_back(s);
	}

	template <typename T>
	inline void push_allocated_image(const T& image) {
		allocatedImages.push_back(image);
	}

	template <typename T>
	inline void push_offscreen_image(const T& image) {
		offscreenImages.push_back(image);
	}

	template <typename T>
	inline void push_frame_allocated_buffer(const T& buffer) {
		vmaFrameAllocatedBuffer.push_back(buffer);
	}

	template <typename T>
	inline void push_allocated_buffer(const T& buffer) {
		vmaAllocatedBuffer.push_back(buffer);
	}

	template <typename T>
	inline void push_descriptor_set_layout(T layout) {
		descriptorSetLayouts.push_back(layout);
	}

	template <typename T>
	inline void push_descriptor_pool(T pool) {
		descriptorPools.push_back(pool);
	}
	template <typename T>
	inline void push_command_pool(T pool) {
		commandPool.push_back(pool);
	}
	template <typename T>
	inline void push_fence(T fence) {
		fences.push_back(fence);
	}
	template <typename T>
	inline void push_renderpass(T rp) {
		renderpass.push_back(rp);
	}
	template <typename T>
	inline void push_framebuffer(T fb) {
		framebuffer.push_back(fb);
	}
	template <typename T>
	inline void push_pipeline(T pipe) {
		pipelines.push_back(pipe);
	}
	template <typename T>
	inline void push_pipeline_layout(T layout) {
		pipelineLayouts.push_back(layout);
	}

	template <typename T>
	void push_mesh_buffer_deletion(T& mesh) {
		// If mesh is a pointer/smart pointer, use ->meshBuffers
		if constexpr (requires { mesh->meshBuffers; }) {
			vmaAllocatedBuffer.push_back(mesh->meshBuffers.indexBuffer);
			vmaAllocatedBuffer.push_back(mesh->meshBuffers.vertexBuffer);
		}
		// Otherwise, assume mesh is already a GPUMeshBuffers object
		else {
			vmaAllocatedBuffer.push_back(mesh.indexBuffer);
			vmaAllocatedBuffer.push_back(mesh.vertexBuffer);
		}
	}

private:
	std::vector<AllocatedBuffer> vmaFrameAllocatedBuffer;
	std::vector<AllocatedBuffer> vmaAllocatedBuffer;
	std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
	std::vector<VkDescriptorPool> descriptorPools;
	std::vector<VkSampler> samplers;
	std::vector<AllocatedImage> offscreenImages;
	std::vector<AllocatedImage> allocatedImages;
	std::vector<VkCommandPool> commandPool;
	std::vector<VkFence> fences;
	std::vector<VkRenderPass> renderpass;
	std::vector<VkFramebuffer> framebuffer;
	std::vector<VkPipelineLayout> pipelineLayouts;
};

