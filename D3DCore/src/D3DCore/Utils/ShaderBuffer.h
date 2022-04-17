#pragma once
#include <DirectXMath.h>
#include <memory>
#include <vector>
#include <string>

#define LEAF_ELEMENT_TYPES \
		X(Float) \
		X(Float2) \
		X(Float3) \
		X(Float4) \
		X(Int) \
		X(Uint) \
		X(Float4x4) \
		X(Bool) 

namespace d3dcore::utils::shader
{
	enum class ShaderType
	{
#define X(element) element,
		LEAF_ELEMENT_TYPES
#undef X
		Struct,
		Array,
		Empty
	};

#define SHADER_TYPE_MAP_VALID_FIELDS(inSysType, inHlslSize) \
		using SysType = inSysType; \
		static constexpr bool valid = true; \
		static constexpr size_t hlslSize = inHlslSize; 

	template<ShaderType Type>
	struct ShaderTypeMap
	{
		static constexpr bool valid = false;
	};

	template<>
	struct ShaderTypeMap<ShaderType::Float>
	{
		SHADER_TYPE_MAP_VALID_FIELDS(float, sizeof(float))
	};

	template<>
	struct ShaderTypeMap<ShaderType::Float2>
	{
		SHADER_TYPE_MAP_VALID_FIELDS(DirectX::XMFLOAT2, sizeof(DirectX::XMFLOAT2))
	};

	template<>
	struct ShaderTypeMap<ShaderType::Float3>
	{
		SHADER_TYPE_MAP_VALID_FIELDS(DirectX::XMFLOAT3, sizeof(DirectX::XMFLOAT3))
	};

	template<>
	struct ShaderTypeMap<ShaderType::Float4>
	{
		SHADER_TYPE_MAP_VALID_FIELDS(DirectX::XMFLOAT4, sizeof(DirectX::XMFLOAT4))
	};

	template<>
	struct ShaderTypeMap<ShaderType::Int>
	{
		SHADER_TYPE_MAP_VALID_FIELDS(int32_t, sizeof(int32_t))
	};

	template<>
	struct ShaderTypeMap<ShaderType::Uint>
	{
		SHADER_TYPE_MAP_VALID_FIELDS(uint32_t, sizeof(uint32_t))
	};

	template<>
	struct ShaderTypeMap<ShaderType::Float4x4>
	{
		SHADER_TYPE_MAP_VALID_FIELDS(DirectX::XMFLOAT4X4, sizeof(DirectX::XMFLOAT4X4))
	};

	template<>
	struct ShaderTypeMap<ShaderType::Bool>
	{
		SHADER_TYPE_MAP_VALID_FIELDS(bool, sizeof(int))
	};

	template<typename T>
	struct ShaderTypeReverseMap
	{
		static constexpr bool valid = false;
	};

#define X(element) \
template<> \
struct ShaderTypeReverseMap<typename ShaderTypeMap<ShaderType::element>::SysType> \
{ \
	static constexpr ShaderType type = ShaderType::element; \
	static constexpr bool valid = true; \
};
	LEAF_ELEMENT_TYPES
#undef X

		class BufferLayoutElement
	{
	private:
		struct ExtraDataBase
		{
			virtual ~ExtraDataBase() = default;
		};

	public:
		bool Exists() const;
		//structs

		BufferLayoutElement& Add(ShaderType type, const std::string& name);

		template<ShaderType Type>
		BufferLayoutElement& Add(const std::string& name)
		{
			return Add(Type, name);
		}

		// arrays

		void SetArray(ShaderType type, size_t count);
		template<ShaderType Type>
		void SetArray(size_t count)
		{
			SetArray(Type, count);
		}
		BufferLayoutElement& GetArrayStruct();
		// get the offset of element in bytes and element
		std::pair<size_t, const BufferLayoutElement*> GetArrayElement(size_t index) const;

		size_t GetOffsetBegin() const; // only work after Finalize
		size_t GetOffsetEnd() const; // only work after Finalize

		size_t GetSizeInBytes() const;

		BufferLayoutElement& operator[](const std::string& name);
		const BufferLayoutElement& operator[](const std::string& name) const;

		// returns offset of leaf types for read/write purposes w/ typecheck in Debug
		template<typename T>
		size_t Resolve() const
		{
			switch (m_type)
			{
#define X(element) case ShaderType::element: assert(typeid(T) == typeid(ShaderTypeMap<ShaderType::element>::SysType)); return m_offset;
				LEAF_ELEMENT_TYPES
#undef X
			default:
				assert(false && "Tried to resolve non-leaf/empty/invalid element");
				return 0;
			}
		}

	private:
		BufferLayoutElement(ShaderType type, bool align16);
		BufferLayoutElement() = default;

		size_t Finalize(size_t offset);
		size_t FinalizeStruct(size_t offset);
		size_t FinalizeArray(size_t offset);

		static size_t AdvanceTo16BytesBoundary(size_t offset);
		static bool Crosses16BytesBoundary(size_t offset, size_t size);
		static size_t AdvanceIfCrosses16BytesBoundary(size_t offset, size_t size);

		static bool IsFieldNameAllowed(const std::string& name);
		static BufferLayoutElement& GetEmptyElement();

		ShaderType m_type = ShaderType::Empty;
		std::unique_ptr<ExtraDataBase> m_extraData = nullptr;
		size_t m_offset = 0;
		bool m_align16 = false;
		bool m_finalize = false;

		friend struct ExtraData;
		friend class BufferLayout;
	};

	class BufferLayout
	{
	public:
		BufferLayout(bool align16);

		template<ShaderType Type>
		BufferLayoutElement& Add(const std::string& name)
		{
			return m_rootElement->Add<Type>(name);
		}

		size_t GetSizeInBytes() const;
		std::shared_ptr<BufferLayoutElement> DeliverRoot();

		BufferLayoutElement& operator[](const std::string& name);

	private:
		std::shared_ptr<BufferLayoutElement> m_rootElement = nullptr;
		bool m_align16 = false;

		friend class Buffer;
	};

	class ConstBufferElement
	{
	public:
		// this is a proxy type emitted when you use addressof& on the Ref
		// it allows conversion to pointer type, useful for using Buffer
		// elements with ImGui widget functions etc.
		class Ptr
		{
		public:
			// conversion to read/write pointer to supported SysType
			template<typename T>
			operator const T* () const
			{
				static_assert(ShaderTypeReverseMap<std::remove_const_t<T>>::valid, "Unsupported SysType used in pointer conversion");
				return &static_cast<const T&>(*m_ref);
			}

		private:
			Ptr(const ConstBufferElement* ref) : m_ref(ref) {}
			const ConstBufferElement* m_ref;

			friend class ConstBufferElement;
		};

	public:
		bool Exists() const
		{
			return m_layoutElement->Exists();
		}

		ConstBufferElement operator[](const std::string& name) const
		{
			return { m_bytes, &(*m_layoutElement)[name], 0 };
		}

		ConstBufferElement operator[](size_t index) const
		{
			const auto element = m_layoutElement->GetArrayElement(index);
			return { m_bytes, element.second, element.first };
		}

		Ptr operator&() const
		{
			return { this };
		}

		template<typename T>
		operator const T& () const
		{
			static_assert(ShaderTypeReverseMap<std::remove_const_t<T>>::valid, "Unsupported SysType used in conversion");
			return *reinterpret_cast<const T*>(m_bytes + m_offset + m_layoutElement->Resolve<T>());
		}

	private:
		ConstBufferElement(const char* bytes, const BufferLayoutElement* layoutElement, size_t offset)
			: m_bytes(bytes), m_layoutElement(layoutElement), m_offset(offset)
		{
		}

		const char* m_bytes = nullptr;
		const BufferLayoutElement* m_layoutElement = nullptr;
		size_t m_offset = 0; // for array

		friend class Buffer;
		friend class BufferElement;
	};

	class BufferElement
	{
	public:
		// this is a proxy type emitted when you use addressof& on the Ref
		// it allows conversion to pointer type, useful for using Buffer
		// elements with ImGui widget functions etc.
		class Ptr
		{
		public:
			// conversion to read/write pointer to supported SysType
			template<typename T>
			operator T* () const
			{
				static_assert(ShaderTypeReverseMap<std::remove_const_t<T>>::valid, "Unsupported SysType used in pointer conversion");
				return &static_cast<T&>(*m_ref);
			}

		private:
			Ptr(BufferElement* ref) : m_ref(ref) {}
			BufferElement* m_ref;

			friend class BufferElement;
		};

	public:
		bool Exists() const
		{
			return m_layoutElement->Exists();
		}

		operator ConstBufferElement() const
		{
			return { m_bytes, m_layoutElement, m_offset };
		}

		BufferElement operator[](const std::string& name) const
		{
			return { m_bytes, &(*m_layoutElement)[name], m_offset };
		}

		BufferElement operator[](size_t index) const
		{
			const auto element = m_layoutElement->GetArrayElement(index);
			return { m_bytes, element.second, element.first };
		}

		Ptr operator&() const
		{
			return { const_cast<BufferElement*>(this) };
		}

		template<typename T>
		operator T& () const
		{
			static_assert(ShaderTypeReverseMap<std::remove_const_t<T>>::valid, "Unsupported SysType used in converstion");
			return *reinterpret_cast<T*>(m_bytes + m_offset + m_layoutElement->Resolve<T>());
		}

		template<typename T>
		T& operator=(const T& rhs)
		{
			static_assert(ShaderTypeReverseMap< std::remove_const_t<T>>::valid, "Unsupported SysType used in converstion");
			return static_cast<T&>(*this) = rhs;
		}

	private:
		BufferElement(char* bytes, const BufferLayoutElement* layoutElement, size_t offset)
			: m_bytes(bytes), m_layoutElement(layoutElement), m_offset(offset)
		{
		}

		char* m_bytes = nullptr;
		const BufferLayoutElement* m_layoutElement = nullptr;
		size_t m_offset = 0; // for array

		friend class Buffer;
	};

	class Buffer
	{
	public:
		Buffer(const std::shared_ptr<BufferLayoutElement>& rootElement);
		Buffer() = default;
		//Buffer(const Buffer&) = delete;
		//Buffer& operator=(const Buffer&) = delete;

		BufferElement operator[](const std::string& name);
		ConstBufferElement operator[](const std::string& name) const;
		const char* GetData() const;
		size_t GetSizeInBytes() const;

		void SetLayout(const std::shared_ptr<BufferLayoutElement>& rootElement);
		const std::shared_ptr<BufferLayoutElement>& ShareLayout() const;

	private:
		std::shared_ptr<BufferLayoutElement> m_rootElement = nullptr;
		std::vector<char> m_bytes;
	};
}

#ifndef BUFFER_IMPL_SRC
#undef LEAF_ELEMENT_TYPES
#endif