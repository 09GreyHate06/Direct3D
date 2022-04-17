#include "d3dcpch.h"
#define BUFFER_IMPL_SRC
#include "ShaderBuffer.h"

namespace d3dcore::utils::shader
{
	struct ExtraData
	{
		struct Struct : public BufferLayoutElement::ExtraDataBase
		{
			std::vector<std::pair<std::string, BufferLayoutElement>> fields;
		};

		struct Array : public BufferLayoutElement::ExtraDataBase
		{
			std::optional<BufferLayoutElement> element;
			size_t count = 0;
		};
	};

	BufferLayoutElement::BufferLayoutElement(ShaderType type, bool align16)
		: m_type(type), m_align16(align16)
	{
		if (m_type == ShaderType::Struct)
			m_extraData = std::make_unique<ExtraData::Struct>();
		else if (m_type == ShaderType::Array)
			m_extraData = std::make_unique<ExtraData::Array>();
	}

	bool BufferLayoutElement::Exists() const
	{
		return m_type != ShaderType::Empty;
	}

	BufferLayoutElement& BufferLayoutElement::Add(ShaderType type, const std::string& name)
	{
		assert(!m_finalize && "Can't add element on finalized layout");
		assert(m_type == ShaderType::Struct && "Adding to a non struct element");
		assert(IsFieldNameAllowed(name) && "Ivalid Symbol name in struct");
		auto& structData = static_cast<ExtraData::Struct&>(*m_extraData);
		auto it = std::find_if(structData.fields.begin(), structData.fields.end(), [&](const auto& field)
			{
				return field.first == name;
			});

		assert(it == structData.fields.end() && "Adding duplicate name");
		return structData.fields.emplace_back(name, BufferLayoutElement(type, m_align16)).second;
	}

	void BufferLayoutElement::SetArray(ShaderType type, size_t count)
	{
		assert(!m_finalize && "Can't set element on finalized layout");
		assert(m_type == ShaderType::Array && "Setting Array of a non array element");
		assert(count != 0);
		auto& arrayData = static_cast<ExtraData::Array&>(*m_extraData);
		arrayData.element = { type, m_align16 };
		arrayData.count = count;
	}

	BufferLayoutElement& BufferLayoutElement::GetArrayStruct()
	{
		auto& extraData = static_cast<ExtraData::Array&>(*m_extraData);
		assert(m_type == ShaderType::Array && extraData.element->m_type == ShaderType::Struct && "Array data is not a struct");
		return *extraData.element;
	}

	std::pair<size_t, const BufferLayoutElement*> BufferLayoutElement::GetArrayElement(size_t index) const
	{
		assert(m_finalize && "Indexing on unfinalized layout");
		assert(m_type == ShaderType::Array && "Indexing to a non-array element");
		const auto& data = static_cast<ExtraData::Array&>(*m_extraData);
		assert(index < data.count);
		return { (m_align16 ? AdvanceTo16BytesBoundary(data.element->GetSizeInBytes()) : data.element->GetSizeInBytes()) * index, &(*data.element) };
	}

	size_t BufferLayoutElement::GetOffsetBegin() const
	{
		assert(m_finalize);
		return m_offset;
	}

	size_t BufferLayoutElement::GetOffsetEnd() const
	{
		assert(m_finalize);
		switch (m_type)
		{
#define X(element) case ShaderType::element: return m_offset + ShaderTypeMap<ShaderType::element>::hlslSize;
			LEAF_ELEMENT_TYPES
#undef X
		case ShaderType::Struct:
			{
				const auto& data = static_cast<ExtraData::Struct&>(*m_extraData);
				return m_align16 ? AdvanceTo16BytesBoundary(data.fields.back().second.GetOffsetEnd()) : data.fields.back().second.GetOffsetEnd();
			}
		case ShaderType::Array:
		{
			const auto& data = static_cast<ExtraData::Array&>(*m_extraData);
			return m_offset + (m_align16 ? AdvanceTo16BytesBoundary(data.element->GetSizeInBytes()) :
				data.element->GetSizeInBytes()) * data.count;
		}
		default:
			assert(false && "Tried to get offset of empty or invalid element");
			return 0;
		}
	}

	size_t BufferLayoutElement::GetSizeInBytes() const
	{
		assert(m_finalize);
		return GetOffsetEnd() - GetOffsetBegin();
	}

	BufferLayoutElement& BufferLayoutElement::operator[](const std::string& name)
	{
		assert(m_type == ShaderType::Struct && "Keying to a non-struct element");
		auto& structData = static_cast<ExtraData::Struct&>(*m_extraData);

		auto it = std::find_if(structData.fields.begin(), structData.fields.end(), [&](auto& field)
			{
				return field.first == name;
			});

		return it != structData.fields.end() ? it->second : GetEmptyElement();
	}

	const BufferLayoutElement& BufferLayoutElement::operator[](const std::string& name) const
	{
		return const_cast<BufferLayoutElement&>(*this)[name];
	}

	size_t BufferLayoutElement::Finalize(size_t offset)
	{
		assert(!m_finalize && "Already finalized");
		m_finalize = true;

		switch (m_type)
		{
#define X(element) \
	case ShaderType::element: \
	m_offset = m_align16 ? AdvanceIfCrosses16BytesBoundary(offset, ShaderTypeMap<ShaderType::element>::hlslSize) : offset; \
	return m_offset + ShaderTypeMap<ShaderType::element>::hlslSize; 

			LEAF_ELEMENT_TYPES
#undef X
		case ShaderType::Struct:
			return FinalizeStruct(offset);
		case ShaderType::Array:
			return FinalizeArray(offset);
		default:
			assert(false && "Bad type in size computation");
			return 0;
		}
	}

	size_t BufferLayoutElement::FinalizeStruct(size_t offset)
	{
		auto& data = static_cast<ExtraData::Struct&>(*m_extraData);
		m_offset = m_align16 ? AdvanceTo16BytesBoundary(offset) : offset;
		size_t offsetNext = m_offset;
		std::for_each(data.fields.begin(), data.fields.end(), [&](auto& field)
			{
				offsetNext = field.second.Finalize(offsetNext);
			});

		return offsetNext;
	}

	size_t BufferLayoutElement::FinalizeArray(size_t offset)
	{
		auto& data = static_cast<ExtraData::Array&>(*m_extraData);
		assert(data.count > 0);
		m_offset = m_align16 ? AdvanceTo16BytesBoundary(offset) : offset;
		data.element->Finalize(m_offset);
		return GetOffsetEnd();
	}

	inline size_t BufferLayoutElement::AdvanceTo16BytesBoundary(size_t offset)
	{
		return offset + (16 - offset % 16) % 16;
	}

	inline bool BufferLayoutElement::Crosses16BytesBoundary(size_t offset, size_t size)
	{
		const size_t end = offset + size;
		const size_t pageStart = offset / 16;
		const size_t pageEnd = end / 16;
		return (pageStart != pageEnd && end % 16 != 0) || size > 16;
	}

	inline size_t BufferLayoutElement::AdvanceIfCrosses16BytesBoundary(size_t offset, size_t size)
	{
		return Crosses16BytesBoundary(offset, size) ? AdvanceTo16BytesBoundary(offset) : offset;
	}

	bool BufferLayoutElement::IsFieldNameAllowed(const std::string& name)
	{
		// symbols can contain alphanumeric and underscore, must not start with digit
		return !name.empty() && !std::isdigit(name.front()) &&
			std::all_of(name.begin(), name.end(), [](char c)
				{
					return std::isalnum(c) || c == '_';
				});
	}

	BufferLayoutElement& BufferLayoutElement::GetEmptyElement()
	{
		static BufferLayoutElement empty;
		return empty;
	}








	BufferLayout::BufferLayout(bool align16)
		: m_align16(align16)
	{
		m_rootElement = std::shared_ptr<BufferLayoutElement>(new BufferLayoutElement(ShaderType::Struct, m_align16));
	}

	size_t BufferLayout::GetSizeInBytes() const
	{
		return m_rootElement->GetSizeInBytes();
	}

	std::shared_ptr<BufferLayoutElement> BufferLayout::DeliverRoot()
	{
		auto temp = std::move(m_rootElement);
		temp->Finalize(0);
		*this = BufferLayout(m_align16);
		return std::move(temp);
	}

	BufferLayoutElement& BufferLayout::operator[](const std::string& name)
	{
		return (*m_rootElement)[name];
	}




	Buffer::Buffer(const std::shared_ptr<BufferLayoutElement>& rootElement)
		: m_rootElement(rootElement), m_bytes(rootElement->GetOffsetEnd())
	{
	}

	BufferElement Buffer::operator[](const std::string& name)
	{
		return { m_bytes.data(), &(*m_rootElement)[name], 0 };
	}

	ConstBufferElement Buffer::operator[](const std::string& name) const
	{
		return { m_bytes.data(), &(*m_rootElement)[name], 0 };
	}

	const char* Buffer::GetData() const
	{
		return m_bytes.data();
	}

	size_t Buffer::GetSizeInBytes() const
	{
		return m_bytes.size();
	}

	void Buffer::SetLayout(const std::shared_ptr<BufferLayoutElement>& rootElement)
	{
		m_rootElement = rootElement;
		m_bytes = std::vector<char>(m_rootElement->GetOffsetEnd());
	}

	const std::shared_ptr<BufferLayoutElement>& Buffer::ShareLayout() const
	{
		return m_rootElement;
	}

}