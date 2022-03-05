#pragma once
#include "Base.h"

extern d3dcore::Application* d3dcore::CreateApplication();

int main()
{
	try
	{
		d3dcore::Log::Init();
		auto app = d3dcore::CreateApplication();
		app->Run();
		delete app;
	}
	catch (const d3dcore::D3DCException& e)
	{
		MessageBoxA(nullptr, e.what(), e.GetType(), MB_OK | MB_ICONERROR);
	}
	catch (const std::exception& e)
	{
		MessageBoxA(nullptr, e.what(), "Standard Exception", MB_OK | MB_ICONERROR);
	}
	catch (...)
	{
		MessageBoxA(nullptr, "No details available", "Unknown Exception", MB_OK | MB_ICONERROR);
	}

	return 0;
}