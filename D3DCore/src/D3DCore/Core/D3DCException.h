#pragma once
#include <exception>
#include <string>

namespace d3dcore
{
	class D3DCException : std::exception
	{
	public:
		D3DCException(int line, const std::string& file)
			: m_line(line), m_file(file) { }

		virtual const char* what() const override;
		virtual const char* GetType() const { return "D3DCException"; }

		std::string GetOriginString() const;
		int GetLine() const { return m_line; }
		const std::string& GetFile() const { return m_file; }

	protected:
		mutable std::string m_whatBuffer;

	private:
		int m_line;
		std::string m_file;
	};
}