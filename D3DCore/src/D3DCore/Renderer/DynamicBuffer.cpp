#include "d3dcpch.h"
//#define DCB_IMPL_SOURCE
//#include "DynamicBuffer.h"
//
//namespace d3dcore
//{
//	struct ExtraData
//	{
//		struct Struct : public BufferLayoutElement::ExtraDataBase
//		{
//			std::vector<std::pair<std::string, BufferLayoutElement>> fields;
//		};
//
//		struct Array : public BufferLayoutElement::ExtraDataBase
//		{
//			std::optional<BufferLayoutElement> element;
//			size_t count = 0;
//		};
//	};
//
//	BufferLayoutElement::BufferLayoutElement(ShaderType type, bool align16)
//		: m_type(type), m_align16(align16)
//	{
//		if (m_type == ShaderType::Struct)
//			m_extraData = std::make_unique<ExtraData::Struct>();
//		else if (m_type == ShaderType::Array)
//			m_extraData = std::make_unique<ExtraData::Array>();
//	}
//
//	bool BufferLayoutElement::Exists() const
//	{
//		return m_type != ShaderType::Empty;
//	}
//
//	BufferLayoutElement& BufferLayoutElement::Add(ShaderType type, const std::string& name)
//	{
//		assert(m_type == ShaderType::Struct && "Adding to a non struct element");
//		assert(IsFieldNameAllowed(name) && "Ivalid Symbol name in struct");
//		auto& structData = static_cast<ExtraData::Struct&>(*m_extraData);
//		auto it = std::find_if(structData.fields.begin(), structData.fields.end(), [&](const auto& field)
//			{
//				return field.first == name;
//			});
//
//		assert(it == structData.fields.end() && "Adding duplicate name");
//		return structData.fields.emplace_back(name, BufferLayoutElement(type, m_align16)).second;
//	}
//
//	void BufferLayoutElement::SetArray(ShaderType type, size_t count)
//	{
//		assert(m_type == ShaderType::Array && "Setting Array of a non array element");
//		assert(count != 0);
//		auto& arrayData = static_cast<ExtraData::Array&>(*m_extraData);
//		arrayData.element = { type, m_align16 };
//		arrayData.count = count;
//	}
//
//	BufferLayoutElement& BufferLayoutElement::GetArrayStruct()
//	{
//		auto& extraData = static_cast<ExtraData::Array&>(*m_extraData);
//		assert(m_type == ShaderType::Array && extraData.element->m_type == ShaderType::Struct && "Array data is not a struct");
//		return *extraData.element;
//	}
//
//	std::pair<size_t, const BufferLayoutElement*> BufferLayoutElement::GetArrayElement(size_t index) const
//	{
//		assert(m_type == ShaderType::Array && "Indexing to a non-array element");
//		const auto& data = static_cast<ExtraData::Array&>(*m_extraData);
//		assert(index < data.count);
//		return { (m_align16 ? AdvanceTo16BytesBoundary(data.element->GetSizeInBytes()) : data.element->GetSizeInBytes()) * index, &(*data.element) };
//	}
//
//	size_t BufferLayoutElement::GetOffsetBegin() const
//	{
//		return m_offset;
//	}
//
//	size_t BufferLayoutElement::GetOffsetEnd() const
//	{
//		switch (m_type)
//		{
//#define X(element) case ShaderType::element: return m_offset + ShaderTypeMap<ShaderType::element>::hlslSize;
//			LEAF_ELEMENT_TYPES
//#undef X
//		case ShaderType::Struct:
//			{
//				const auto& data = static_cast<ExtraData::Struct&>(*m_extraData);
//				return m_align16 ? AdvanceTo16BytesBoundary(data.fields.back().second.GetOffsetEnd()) : data.fields.back().second.GetOffsetEnd();
//			}
//		case ShaderType::Array:
//		{
//			const auto& data = static_cast<ExtraData::Array&>(*m_extraData);
//			return m_offset + (m_align16 ? AdvanceTo16BytesBoundary(data.element->GetSizeInBytes()) :
//				data.element->GetSizeInBytes()) * data.count;
//		}
//		default:
//			assert(false && "Tried to get offset of empty or invalid element");
//			return 0;
//		}
//	}
//
//	size_t BufferLayoutElement::GetSizeInBytes() const
//	{
//		return GetOffsetEnd() - GetOffsetBegin();
//	}
//
//	BufferLayoutElement& BufferLayoutElement::operator[](const std::string& name)
//	{
//		assert(m_type == ShaderType::Struct && "Keying to a non-struct element");
//		auto& structData = static_cast<ExtraData::Struct&>(*m_extraData);
//
//		auto it = std::find_if(structData.fields.begin(), structData.fields.end(), [&](auto& field)
//			{
//				return field.first == name;
//			});
//
//		return it != structData.fields.end() ? it->second : GetEmptyElement();
//	}
//
//	const BufferLayoutElement& BufferLayoutElement::operator[](const std::string& name) const
//	{
//		return const_cast<BufferLayoutElement&>(*this)[name];
//	}
//
//	size_t BufferLayoutElement::Finalize(size_t offset)
//	{
//		switch (m_type)
//		{
//#define X(element) \
//	case ShaderType::element: \
//	m_offset = m_align16 ? AdvanceIfCrosses16BytesBoundary(offset, ShaderTypeMap<ShaderType::element>::hlslSize) : offset; \
//	return m_offset + ShaderTypeMap<ShaderType::element>::hlslSize; 
//
//			LEAF_ELEMENT_TYPES
//#undef X
//		case ShaderType::Struct:
//			return FinalizeStruct(offset);
//		case ShaderType::Array:
//			return FinalizeArray(offset);
//		default:
//			assert(false && "Bad type in size computation");
//			return 0;
//		}
//	}
//
//	size_t BufferLayoutElement::FinalizeStruct(size_t offset)
//	{
//		auto& data = static_cast<ExtraData::Struct&>(*m_extraData);
//		m_offset = m_align16 ? AdvanceTo16BytesBoundary(offset) : offset;
//		size_t offsetNext = m_offset;
//		std::for_each(data.fields.begin(), data.fields.end(), [&](auto& field)
//			{
//				offsetNext = field.second.Finalize(offsetNext);
//			});
//
//		return offsetNext;
//	}
//
//	size_t BufferLayoutElement::FinalizeArray(size_t offset)
//	{
//		auto& data = static_cast<ExtraData::Array&>(*m_extraData);
//		m_offset = m_align16 ? AdvanceTo16BytesBoundary(offset) : offset;
//		data.element->Finalize(m_offset);
//		return GetOffsetEnd();
//	}
//
//	size_t BufferLayoutElement::AdvanceTo16BytesBoundary(size_t offset)
//	{
//		return offset + (16 - offset % 16) % 16;
//	}
//
//	bool BufferLayoutElement::Crosses16BytesBoundary(size_t offset, size_t size)
//	{
//		const size_t end = offset + size;
//		const size_t pageStart = offset / 16;
//		const size_t pageEnd = end / 16;
//		return (pageStart != pageEnd && end % 16 != 0) || size > 16;
//	}
//
//	inline size_t BufferLayoutElement::AdvanceIfCrosses16BytesBoundary(size_t offset, size_t size)
//	{
//		return Crosses16BytesBoundary(offset, size) ? AdvanceTo16BytesBoundary(offset) : offset;
//	}
//
//	bool BufferLayoutElement::IsFieldNameAllowed(const std::string& name)
//	{
//		// symbols can contain alphanumeric and underscore, must not start with digit
//		return !name.empty() && !std::isdigit(name.front()) &&
//			std::all_of(name.begin(), name.end(), [](char c)
//				{
//					return std::isalnum(c) || c == '_';
//				});
//	}
//
//	BufferLayoutElement& BufferLayoutElement::GetEmptyElement()
//	{
//		static BufferLayoutElement empty;
//		return empty;
//	}
//
//
//
//
//
//
//
//
//	BufferLayout::BufferLayout(bool align16)
//	{
//		m_rootElement = std::shared_ptr<BufferLayoutElement>(new BufferLayoutElement(ShaderType::Struct, align16));
//	}
//
//	size_t BufferLayout::GetSizeInBytes() const
//	{
//		return m_rootElement->GetSizeInBytes();
//	}
//
//	void BufferLayout::Finalize()
//	{
//		m_rootElement->Finalize(0);
//	}
//
//	BufferLayoutElement& BufferLayout::operator[](const std::string& name)
//	{
//		return (*m_rootElement)[name];
//	}
//
//
//
//	ConstBufferElement::ConstBufferElement(const char* bytes, const BufferLayoutElement* layoutElement, size_t offset)
//		: m_bytes(bytes), m_layoutElement(layoutElement), m_offset(offset)
//	{
//	}
//
//	bool ConstBufferElement::Exists() const
//	{
//		return m_layoutElement->Exists();
//	}
//
//	ConstBufferElement ConstBufferElement::operator[](const std::string& name) const
//	{
//		return { m_bytes, &(*m_layoutElement)[name], 0 };
//	}
//
//	ConstBufferElement ConstBufferElement::operator[](size_t index) const
//	{
//		const auto element = m_layoutElement->GetArrayElement(index);
//		return { m_bytes, element.second, element.first };
//	}
//
//	ConstBufferElement::Ptr ConstBufferElement::operator&() const
//	{
//		return { this };
//	}
//
//
//
//	BufferElement::BufferElement(char* bytes, const BufferLayoutElement* layoutElement, size_t offset)
//		: m_bytes(bytes), m_layoutElement(layoutElement), m_offset(offset)
//	{
//	}
//
//	bool BufferElement::Exists() const
//	{
//		return m_layoutElement->Exists();
//	}
//
//	BufferElement::operator ConstBufferElement() const
//	{
//		return { m_bytes, m_layoutElement, m_offset };
//	}
//
//	BufferElement BufferElement::operator[](const std::string& name) const
//	{
//		return { m_bytes, &(*m_layoutElement)[name], m_offset };
//	}
//
//	BufferElement BufferElement::operator[](size_t index) const
//	{
//		const auto element = m_layoutElement->GetArrayElement(index);
//		return { m_bytes, element.second, element.first };
//	}
//
//	BufferElement::Ptr BufferElement::operator&() const
//	{
//		return { const_cast<BufferElement*>(this) };
//	}
//
//
//
//	Buffer::Buffer(const BufferLayout& layout)
//		: m_rootElement(layout.m_rootElement), m_bytes(m_rootElement->GetOffsetEnd())
//	{
//	}
//
//	BufferElement Buffer::operator[](const std::string& name)
//	{
//		return { m_bytes.data(), &(*m_rootElement)[name], 0 };
//	}
//
//	ConstBufferElement Buffer::operator[](const std::string& name) const
//	{
//		return const_cast<Buffer&>(*this)[name];
//	}
//
//	const char* Buffer::GetData() const
//	{
//		return m_bytes.data();
//	}
//
//	size_t Buffer::GetSizeInBytes() const
//	{
//		return m_bytes.size();
//	}
//}