#include "rvpch.h"

#include "Engine/Rendering/Vulkan/VKBuffer.h"
#include "Engine/Rendering/Vulkan/VKUtils.h"

namespace VK {
VkDeviceSize Buffer::GetAlignment(VkDeviceSize instanceSize, VkDeviceSize minOffsetAlignment) {
	if (minOffsetAlignment > 0) {
		return (instanceSize + minOffsetAlignment - 1) & ~(minOffsetAlignment - 1);
	}
	return instanceSize;
}

Buffer::Buffer(
	VkDeviceSize instanceSize,
	u32 instanceCount,
	VkBufferUsageFlags usageFlags,
	VkMemoryPropertyFlags memoryPropertyFlags,
	VkDeviceSize minOffsetAlignment
)
	: _instanceSize{instanceSize}
	, _instanceCount{instanceCount}
	, _usageFlags{usageFlags}
	, _memoryPropertyFlags{memoryPropertyFlags} {
	_alignmentSize = GetAlignment(instanceSize, minOffsetAlignment);
	_bufferSize    = _alignmentSize * instanceCount;
	CreateBuffer(_bufferSize, usageFlags, memoryPropertyFlags, _buffer, _memory);
}

Buffer::Buffer(size_t size, Buffer::BufferUsage bufferUsage) {
	switch (bufferUsage) {
		case Buffer::BufferUsage::UNIFORM_BUFFER_VISIBLE_TO_CPU: {
			_instanceSize        = size;
			_instanceCount       = 1;
			_usageFlags          = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
			_memoryPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;

			VkDeviceSize minOffsetAlignment = Context::Instance->GetProperties().limits.minUniformBufferOffsetAlignment;

			_alignmentSize = GetAlignment(_instanceSize, minOffsetAlignment);
			_bufferSize    = _alignmentSize * _instanceCount;
			CreateBuffer(_bufferSize, _usageFlags, _memoryPropertyFlags, _buffer, _memory);
			break;
		}
		case Buffer::BufferUsage::STORAGE_BUFFER_VISIBLE_TO_CPU: {
			_instanceSize        = size;
			_instanceCount       = 1;
			_usageFlags          = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
			_memoryPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;

			VkDeviceSize minOffsetAlignment = Context::Instance->GetProperties().limits.minUniformBufferOffsetAlignment;

			_alignmentSize = GetAlignment(_instanceSize, minOffsetAlignment);
			_bufferSize    = _alignmentSize * _instanceCount;
			CreateBuffer(_bufferSize, _usageFlags, _memoryPropertyFlags, _buffer, _memory);
			break;
		}
		default: {
			ENGINE_CRITICAL("unrecognized buffer usage");
		}
	}
}

Buffer::~Buffer() {
	Unmap();
	vkDestroyBuffer(Context::Instance->GetLogicalDevice(), _buffer, nullptr);
	vkFreeMemory(Context::Instance->GetLogicalDevice(), _memory, nullptr);
}

VkResult Buffer::Map(VkDeviceSize size, VkDeviceSize offset) {
	ENGINE_ASSERT(_buffer && _memory, "Called map on buffer before create");
	return vkMapMemory(Context::Instance->GetLogicalDevice(), _memory, offset, size, 0, &_mapped);
}

void Buffer::MapBuffer() {
	Map();
}

void Buffer::Unmap() {
	if (_mapped) {
		vkUnmapMemory(Context::Instance->GetLogicalDevice(), _memory);
		_mapped = nullptr;
	}
}

void Buffer::WriteToBuffer(const void* data, VkDeviceSize size, VkDeviceSize offset) {
	ENGINE_ASSERT(_mapped, "Cannot Copy to Unmapped Buffer");

	if (size == VK_WHOLE_SIZE) {
		memcpy(_mapped, data, _bufferSize);
	} else {
		char* memOffset = (char*)_mapped;
		memOffset += offset;
		memcpy(memOffset, data, size);
	}
}

VkResult Buffer::Flush(VkDeviceSize size, VkDeviceSize offset) {
	VkMappedMemoryRange mappedRange = {};
	mappedRange.sType               = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
	mappedRange.memory              = _memory;
	mappedRange.offset              = offset;
	mappedRange.size                = size;
	return vkFlushMappedMemoryRanges(Context::Instance->GetLogicalDevice(), 1, &mappedRange);
}

VkResult Buffer::Invalidate(VkDeviceSize size, VkDeviceSize offset) {
	VkMappedMemoryRange mappedRange = {};
	mappedRange.sType               = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
	mappedRange.memory              = _memory;
	mappedRange.offset              = offset;
	mappedRange.size                = size;
	return vkInvalidateMappedMemoryRanges(Context::Instance->GetLogicalDevice(), 1, &mappedRange);
}

VkDescriptorBufferInfo Buffer::DescriptorInfo(VkDeviceSize size, VkDeviceSize offset) {
	return VkDescriptorBufferInfo{
		_buffer,
		offset,
		size,
	};
}

void Buffer::WriteToIndex(void* data, int index) {
	WriteToBuffer(data, _instanceSize, index * _alignmentSize);
}

VkResult Buffer::FlushIndex(int index) {
	return Flush(_alignmentSize, index * _alignmentSize);
}

VkDescriptorBufferInfo Buffer::DescriptorInfoForIndex(int index) {
	return DescriptorInfo(_alignmentSize, index * _alignmentSize);
}

VkResult Buffer::InvalidateIndex(int index) {
	return Invalidate(_alignmentSize, index * _alignmentSize);
}
}  // namespace VK