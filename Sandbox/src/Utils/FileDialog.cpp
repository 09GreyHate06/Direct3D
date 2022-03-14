#include "FileDialog.h"
#include <Windows.h>
#include "D3DCore/Core/Log.h"
#include "D3DCore/Core/Application.h"
#include <commdlg.h>

std::string FileDialog::OpenFileDialog(const char* filter)
{
	//HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
	//std::string result = std::string();

	//if (SUCCEEDED(hr))
	//{
	//	CComPtr<IFileOpenDialog> fileOpenDialog;
	//	// Create the FileOpenDialog object.
	//	hr = fileOpenDialog.CoCreateInstance(__uuidof(FileOpenDialog));
	//	if (SUCCEEDED(hr))
	//	{
	//		// Show the Open dialog box.
	//		fileOpenDialog->SetFilter()
	//		hr = fileOpenDialog->Show(d3dcore::Application::Get().GetWindow().GetNativeWindow());

	//		// Get the file name from the dialog box.
	//		if (SUCCEEDED(hr))
	//		{
	//			CComPtr<IShellItem> item;
	//			hr = fileOpenDialog->GetResult(&item);
	//			if (SUCCEEDED(hr))
	//			{
	//				PWSTR pFilePath;
	//				hr = item->GetDisplayName(SIGDN_FILESYSPATH, &pFilePath);
	//				if (SUCCEEDED(hr))
	//				{
	//					std::wstring wFilePath = pFilePath;
	//					result = std::string(wFilePath.begin(), wFilePath.end());
	//					CoTaskMemFree(pFilePath);
	//				}
	//			}
	//		}
	//	}
	//}
	//else
	//{
	//	D3DC_LOG_WARN("[FileDialog]: 'CoinitializeEx' Failed");
	//}

	std::string result = std::string();

	OPENFILENAMEA ofn = {};
	CHAR szFile[260] = { '\0' };
	CHAR currentDir[256] = { '\0' };
	ZeroMemory(&ofn, sizeof(OPENFILENAMEA));
	ofn.lStructSize = sizeof(OPENFILENAMEA);
	ofn.hwndOwner = d3dcore::Application::Get().GetWindow().GetNativeWindow();
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);
	if (GetCurrentDirectoryA(256, currentDir))
		ofn.lpstrInitialDir = currentDir;
	ofn.lpstrFilter = filter;
	ofn.nFilterIndex = 1;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

	if (GetOpenFileNameA(&ofn) == TRUE)
		result = ofn.lpstrFile;

	return result;

}
