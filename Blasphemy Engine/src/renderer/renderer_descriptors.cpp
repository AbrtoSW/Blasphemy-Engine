#include "renderer/renderer.h"
#include "descriptors/descriptors.h"
#include "vulkan_backend/vk_backend.h"
#include "vulkan_backend/vk_frame_manager.h"
#include "util/vk_util.h"

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

	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	VK_CHECK(vkCreateSampler(vkBackend.getDevice(), &samplerInfo, nullptr, &drawImageSampler));

	for (std::uint32_t i = 0; i < FRAME_OVERLAP; ++i) {
		drawImageDescriptorSets[i] = mainDescriptorAllocator.allocate(vkBackend.getDevice(), drawImageDescriptorSetLayout);

		DescriptorWriter writer{};
		writer.write_image(0, drawImages[i].imageView, drawImageSampler, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
		writer.update_set(vkBackend.getDevice(), drawImageDescriptorSets[i]);
		writer.clear();
	}
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
