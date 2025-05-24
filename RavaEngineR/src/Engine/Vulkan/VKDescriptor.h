#pragma once

namespace VK {
class DescriptorSetLayout {
   public:
	class Builder {
	   public:
		Builder() = default;

		Builder& AddBinding(u32 binding, VkDescriptorType descriptorType, VkShaderStageFlags stageFlags, u32 count = 1);

		size_t Size() const { return _bindings.size(); }
		Unique<DescriptorSetLayout> Build() const;

	   private:
		std::unordered_map<u32, VkDescriptorSetLayoutBinding> _bindings{};
	};

	DescriptorSetLayout(std::unordered_map<u32, VkDescriptorSetLayoutBinding> bindings);
	~DescriptorSetLayout();

	NO_COPY(DescriptorSetLayout)

	VkDescriptorSetLayout GetDescriptorSetLayout() const { return _descriptorSetLayout; }

   private:
	VkDescriptorSetLayout _descriptorSetLayout;
	std::unordered_map<u32, VkDescriptorSetLayoutBinding> _bindings;

	friend class DescriptorWriter;
};

class DescriptorPool {
   public:
	class Builder {
	   public:
		Builder() = default;

		Builder& AddPoolSize(VkDescriptorType descriptorType, u32 count);
		Builder& SetPoolFlags(VkDescriptorPoolCreateFlags flags);
		Builder& SetMaxSets(u32 count);
		Unique<DescriptorPool> Build() const;

	   private:
		std::vector<VkDescriptorPoolSize> _poolSizes{};
		u32 _maxSets                           = 1000;
		VkDescriptorPoolCreateFlags _poolFlags = 0;
	};

	DescriptorPool(u32 maxSets, VkDescriptorPoolCreateFlags poolFlags, const std::vector<VkDescriptorPoolSize>& poolSizes);
	~DescriptorPool();

	NO_COPY(DescriptorPool)

	bool AllocateDescriptor(const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet& descriptor) const;

	void FreeDescriptors(std::vector<VkDescriptorSet>& descriptors) const;

	VkDescriptorPool GetDescriptorPool() const { return _descriptorPool; }

	void ResetPool();

   private:
	VkDescriptorPool _descriptorPool;

	friend class DescriptorWriter;
};

class DescriptorWriter {
   public:
	DescriptorWriter(DescriptorSetLayout& setLayout, DescriptorPool& pool);

	DescriptorWriter& WriteBuffer(u32 binding, VkDescriptorBufferInfo* bufferInfo);
	DescriptorWriter& WriteImage(u32 binding, VkDescriptorImageInfo* imageInfo);

	bool Build(VkDescriptorSet& set);
	void Overwrite(VkDescriptorSet& set);

   private:
	DescriptorSetLayout& _setLayout;
	DescriptorPool& _pool;
	std::vector<VkWriteDescriptorSet> _writes;
};
}  // namespace VK