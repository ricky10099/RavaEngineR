#pragma once

#include "Engine/System/Buffer.h"

namespace VK {
class Buffer : public RV::Buffer {
   public:
	Buffer(
		VkDeviceSize instanceSize,
		u32 instanceCount,
		VkBufferUsageFlags usageFlags,
		VkMemoryPropertyFlags memoryPropertyFlags,
		VkDeviceSize minOffsetAlignment = 1
	);

	Buffer(size_t size, BufferUsage bufferUsage);

	~Buffer();

	NO_COPY(Buffer)

	VkResult Map(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
	virtual void MapBuffer() override;
	void Unmap();

	void WriteToBuffer(const void* data, VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
	virtual void WriteToBuffer(const void* data) override { WriteToBuffer(data, VK_WHOLE_SIZE, 0); }
	VkResult Flush(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
	virtual bool Flush() override {
		auto result = Flush(VK_WHOLE_SIZE, 0);
		return result == VK_SUCCESS;
	}
	VkDescriptorBufferInfo DescriptorInfo(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
	VkResult Invalidate(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

	void WriteToIndex(void* data, int index);
	VkResult FlushIndex(int index);
	VkDescriptorBufferInfo DescriptorInfoForIndex(int index);
	VkResult InvalidateIndex(int index);

	VkBuffer GetBuffer() const { return _buffer; }
	void* GetMappedMemory() const { return _mapped; }
	u32 GetInstanceCount() const { return _instanceCount; }
	VkDeviceSize GetInstanceSize() const { return _instanceSize; }
	VkDeviceSize GetAlignmentSize() const { return _instanceSize; }
	VkBufferUsageFlags GetUsageFlags() const { return _usageFlags; }
	VkMemoryPropertyFlags GetMemoryPropertyFlags() const { return _memoryPropertyFlags; }
	VkDeviceSize GetBufferSize() const { return _bufferSize; }

   private:
	void* _mapped          = nullptr;
	VkBuffer _buffer       = VK_NULL_HANDLE;
	VkDeviceMemory _memory = VK_NULL_HANDLE;

	VkDeviceSize _bufferSize;
	u32 _instanceCount;
	VkDeviceSize _instanceSize;
	VkDeviceSize _alignmentSize;
	VkBufferUsageFlags _usageFlags;
	VkMemoryPropertyFlags _memoryPropertyFlags;

   private:
	static VkDeviceSize GetAlignment(VkDeviceSize instanceSize, VkDeviceSize minOffsetAlignment);
};
}  // namespace VK