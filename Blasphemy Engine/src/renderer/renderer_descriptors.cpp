#include "renderer/renderer.h"
#include "descriptors/descriptors.h"
#include "vulkan_backend/vk_backend.h"

void Renderer::createDescriptors() {

	std::vector<DescriptorAllocatorGrowable::PoolSizeRatio> sizes
	{
		{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1}
	};

	mainDescriptorAllocator.init(vkBackend.getDevice(), 10, sizes);

	initDrawImageDescriptor();



}

void Renderer::initDrawImageDescriptor() {

	DescriptorLayoutBuilder b{};
	b.add_binding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
	drawImageDescriptorSetLayout = b.build(vkBackend.getDevice(), VK_SHADER_STAGE_FRAGMENT_BIT);
	drawImageDescriptorSet = mainDescriptorAllocator.allocate(vkBackend.getDevice(), drawImageDescriptorSetLayout);

}

void Renderer::initFrameDescriptors() {

	//TODO

	for (int i = 0; i < FRAME_OVERLAP; ++i) {

		std::vector<DescriptorAllocatorGrowable::PoolSizeRatio> frame_sizes
		{
				{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 3 },
		};
	
	}



}