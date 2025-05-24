#include "rvpch.h"

#include "Engine/Vulkan/VKDescriptor.h"
#include "Engine/Vulkan/VKUtils.h"

namespace VK {
// *************** Descriptor Set Layout Builder *********************
DescriptorSetLayout::Builder& DescriptorSetLayout::Builder::AddBinding(
	u32 binding, VkDescriptorType descriptorType, VkShaderStageFlags stageFlags, u32 count
) {
	ENGINE_ASSERT(_bindings.count(binding) == 0, "Binding already in use");
	VkDescriptorSetLayoutBinding layoutBinding{};
	layoutBinding.binding         = binding;
	layoutBinding.descriptorType  = descriptorType;
	layoutBinding.descriptorCount = count;
	layoutBinding.stageFlags      = stageFlags;
	_bindings[binding]            = layoutBinding;
	return *this;
}

std::unique_ptr<DescriptorSetLayout> DescriptorSetLayout::Builder::Build() const {
	return std::make_unique<DescriptorSetLayout>(_bindings);
}

// *************** Descriptor Set Layout *********************
DescriptorSetLayout::DescriptorSetLayout(std::unordered_map<u32, VkDescriptorSetLayoutBinding> bindings)
	: _bindings{bindings} {
	std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings{};
	for (auto& it : bindings) {
		setLayoutBindings.push_back(it.second);
	}

	VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo{};
	descriptorSetLayoutInfo.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptorSetLayoutInfo.bindingCount = static_cast<u32>(setLayoutBindings.size());
	descriptorSetLayoutInfo.pBindings    = setLayoutBindings.data();

	VkResult result = vkCreateDescriptorSetLayout(
		Context::Instance->GetLogicalDevice(), &descriptorSetLayoutInfo, nullptr, &_descriptorSetLayout
	);
	VK_CHECK(result, "Failed to Create Descriptor Set Layout!");
}

DescriptorSetLayout::~DescriptorSetLayout() {
	vkDestroyDescriptorSetLayout(Context::Instance->GetLogicalDevice(), _descriptorSetLayout, nullptr);
}

// *************** Descriptor Pool Builder *********************
DescriptorPool::Builder& DescriptorPool::Builder::AddPoolSize(VkDescriptorType descriptorType, u32 count) {
	_poolSizes.push_back({descriptorType, count});
	return *this;
}

DescriptorPool::Builder& DescriptorPool::Builder::SetPoolFlags(VkDescriptorPoolCreateFlags flags) {
	_poolFlags = flags;
	return *this;
}
DescriptorPool::Builder& DescriptorPool::Builder::SetMaxSets(u32 count) {
	_maxSets = count;
	return *this;
}

std::unique_ptr<DescriptorPool> DescriptorPool::Builder::Build() const {
	return std::make_unique<DescriptorPool>(_maxSets, _poolFlags, _poolSizes);
}

// *************** Descriptor Pool *********************
DescriptorPool::DescriptorPool(
	u32 maxSets, VkDescriptorPoolCreateFlags poolFlags, const std::vector<VkDescriptorPoolSize>& poolSizes
) {
	VkDescriptorPoolCreateInfo descriptorPoolInfo{};
	descriptorPoolInfo.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptorPoolInfo.poolSizeCount = static_cast<u32>(poolSizes.size());
	descriptorPoolInfo.pPoolSizes    = poolSizes.data();
	descriptorPoolInfo.maxSets       = maxSets;
	descriptorPoolInfo.flags         = poolFlags;

	VkResult result =
		vkCreateDescriptorPool(Context::Instance->GetLogicalDevice(), &descriptorPoolInfo, nullptr, &_descriptorPool);
	VK_CHECK(result, "Failed to Create Descriptor Pool!");
}

DescriptorPool::~DescriptorPool() {
	vkDestroyDescriptorPool(Context::Instance->GetLogicalDevice(), _descriptorPool, nullptr);
}

bool DescriptorPool::AllocateDescriptor(const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet& descriptor) const {
	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool     = _descriptorPool;
	allocInfo.pSetLayouts        = &descriptorSetLayout;
	allocInfo.descriptorSetCount = 1;

	VkResult result = vkAllocateDescriptorSets(Context::Instance->GetLogicalDevice(), &allocInfo, &descriptor);
	if (result != VK_SUCCESS) {
		return false;
	}
	return true;
}

void DescriptorPool::FreeDescriptors(std::vector<VkDescriptorSet>& descriptors) const {
	vkFreeDescriptorSets(
		Context::Instance->GetLogicalDevice(), _descriptorPool, static_cast<u32>(descriptors.size()), descriptors.data()
	);
}

void DescriptorPool::ResetPool() {
	vkResetDescriptorPool(Context::Instance->GetLogicalDevice(), _descriptorPool, 0);
}

// *************** Descriptor Writer *********************
DescriptorWriter::DescriptorWriter(DescriptorSetLayout& setLayout, DescriptorPool& pool)
	: _setLayout{setLayout}
	, _pool{pool} {}

DescriptorWriter& DescriptorWriter::WriteBuffer(u32 binding, VkDescriptorBufferInfo* bufferInfo) {
	ENGINE_ASSERT(_setLayout._bindings.count(binding) == 1, "Layout does not contain specified binding!");

	auto& bindingDescription = _setLayout._bindings[binding];

	ENGINE_ASSERT(bindingDescription.descriptorCount == 1, "Binding Single Descriptor Info, but Binding expects multiple!");

	VkWriteDescriptorSet write{};
	write.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write.descriptorType  = bindingDescription.descriptorType;
	write.dstBinding      = binding;
	write.pBufferInfo     = bufferInfo;
	write.descriptorCount = 1;

	_writes.push_back(write);
	return *this;
}

DescriptorWriter& DescriptorWriter::WriteImage(u32 binding, VkDescriptorImageInfo* imageInfo) {
	ENGINE_ASSERT(_setLayout._bindings.count(binding) == 1, "Layout does not contain specified binding!");

	auto& bindingDescription = _setLayout._bindings[binding];

	ENGINE_ASSERT(bindingDescription.descriptorCount == 1, "Binding Single Ddescriptor Info, but Binding expects multiple");

	VkWriteDescriptorSet write{};
	write.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write.descriptorType  = bindingDescription.descriptorType;
	write.dstBinding      = binding;
	write.pImageInfo      = imageInfo;
	write.descriptorCount = 1;

	_writes.push_back(write);
	return *this;
}

bool DescriptorWriter::Build(VkDescriptorSet& set) {
	bool success = _pool.AllocateDescriptor(_setLayout.GetDescriptorSetLayout(), set);
	if (!success) {
		return false;
	}
	Overwrite(set);
	return true;
}

void DescriptorWriter::Overwrite(VkDescriptorSet& set) {
	for (auto& write : _writes) {
		write.dstSet = set;
	}
	vkUpdateDescriptorSets(Context::Instance->GetLogicalDevice(), static_cast<u32>(_writes.size()), _writes.data(), 0, nullptr);
}
}  // namespace VK