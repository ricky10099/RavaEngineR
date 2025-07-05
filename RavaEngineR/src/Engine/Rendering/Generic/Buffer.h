#pragma once

namespace RV {
class Buffer {
   public:
	enum class BufferUsage {
		UNIFORM_BUFFER_VISIBLE_TO_CPU,
		STORAGE_BUFFER_VISIBLE_TO_CPU
	};

   public:
	virtual ~Buffer()                            = default;
	virtual void MapBuffer()                     = 0;
	virtual void WriteToBuffer(const void* data) = 0;
	virtual bool Flush()                         = 0;

	static Shared<Buffer> Create(u32 size, BufferUsage bufferUsage = BufferUsage::UNIFORM_BUFFER_VISIBLE_TO_CPU);
};
}  // namespace RV