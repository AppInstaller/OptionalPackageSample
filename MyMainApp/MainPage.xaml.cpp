//
// MainPage.xaml.cpp
// Implementation of the MainPage class.
//

#include "pch.h"
#include "MainPage.xaml.h"

using namespace MyMainApp;

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;
using namespace Windows::UI::Core;


// The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=402352&clcid=0x409


MainPage::MainPage()
{
	InitializeComponent();	
	HookupCatalog();
	
}

void MainPage::ReadOptionalPackageContent()
{
	concurrency::create_task([this]()
	{
		WriteToTextBox("Enumerating Packages");
		auto optionalPackages = EnumerateInstalledPackages();
		for (auto package : optionalPackages)
		{			
			Platform::String^ packageName = ref new String(package->Id->FullName->Data());
			WriteToTextBox(packageName);

			LoadTextFromPackage(package);
			LoadDLLFromPackage(package);
			WriteToTextBox("+++++++++++++++++++");
			WriteToTextBox("\n");
		}
		DebugPrint(L"  Finished loading all optional packages\n");
		DebugPrint(L"\n");
	});
}

std::vector< Windows::ApplicationModel::Package^ > MainPage::EnumerateInstalledPackages()
{
	DebugPrint(L"    Searching for optional packages...\n");

	// Obtain the app's package first to then find all related packages
	auto currentAppPackage = Windows::ApplicationModel::Package::Current;

	// The dependencies list is where the list of optional packages (OP) can be determined
	auto dependencies = currentAppPackage->Dependencies;
	std::vector< Windows::ApplicationModel::Package^ > optionalPackages;

	for (auto package : dependencies)
	{
		//  If it is optional, then add it to our results vector
		if (package->IsOptional)
		{
			DebugPrint(L"    Optional Package found - %ws\n", package->Id->FullName->Data());
			optionalPackages.push_back(package);
		}
	}

	return optionalPackages;    //  Return the resulting vector
}

void MainPage::HookupCatalog()
{
	catalog = Windows::ApplicationModel::PackageCatalog::OpenForCurrentPackage();
	catalog->PackageInstalling += ref new Windows::Foundation::TypedEventHandler<Windows::ApplicationModel::PackageCatalog ^, Windows::ApplicationModel::PackageInstallingEventArgs ^>(this, &MyMainApp::MainPage::OnPackageInstalling);
}

void MainPage::OnPackageInstalling(Windows::ApplicationModel::PackageCatalog ^sender, Windows::ApplicationModel::PackageInstallingEventArgs ^args)
{
	if (args->IsComplete)
	{
		WriteToTextBox("PackageCatalog - Done installing package");
		WriteToTextBox(args->Package->Id->FamilyName->ToString());
	}
}

void MainPage::LoadTextFromPackage(Windows::ApplicationModel::Package^ package)
{
	try
	{
		Windows::Storage::StorageFolder^ installFolder = package->InstalledLocation;		

		concurrency::create_task(installFolder->GetFolderAsync(L"Content"))
			.then([this](concurrency::task< Windows::Storage::StorageFolder^ > result)
		{
			try
			{
				auto assetsFolder = result.get();				

				auto fileAsyncOp = assetsFolder->GetFileAsync(L"SampleFile.txt");

				concurrency::create_task(fileAsyncOp)
					.then([this](concurrency::task< Windows::Storage::StorageFile^ > task)
				{
					try
					{
						auto targetFile = task.get();

						if (targetFile->IsAvailable)
						{
							WriteToTextBox("Found SampleFile.txt - loading contents");
							DebugPrint(L"    %ws is available:\n", targetFile->Name->Data());
							concurrency::create_task(targetFile->OpenAsync(Windows::Storage::FileAccessMode::Read))
								.then([this, targetFile](concurrency::task< Windows::Storage::Streams::IRandomAccessStream^ > task)
							{
								try
								{
									auto readStream = task.get();
									UINT64 const size = readStream->Size;
									if (size <= MAXUINT32)
									{
										auto dataReader = ref new Windows::Storage::Streams::DataReader(readStream);
										concurrency::create_task(dataReader->LoadAsync(static_cast<UINT32>(size)))
											.then([this, targetFile, dataReader](unsigned int numBytesLoaded)
										{
											Platform::String^ fileContent = dataReader->ReadString(numBytesLoaded);												
											
											WriteToTextBox(fileContent);											
											
											delete dataReader; // As a best practice, explicitly close the dataReader resource as soon as it is no longer needed.
											DebugPrint(L"        %ws\n", fileContent->Data());											 
										});
									}
									else
									{
										delete readStream; // As a best practice, explicitly close the readStream resource as soon as it is no longer needed.
										DebugPrint(L"    File %ws is too big for LoadAsync to load in a single chunk. Files larger than 4GB need to be broken into multiple chunks to be loaded by LoadAsync.", targetFile->Name->Data());
									}
								}
								catch (Platform::Exception^ ex)
								{
									DebugPrint(L"    Error 0x%x reading text file\n", ex->HResult);
								}
							}).wait();
						}
						else
						{
							DebugPrint(L"    File not available\n");
						}
					}
					catch (Platform::Exception^ ex)
					{
						DebugPrint(L"   Error 0x%x getting text file\n", ex->HResult);
					}
				}).wait();
			}
			catch (Platform::Exception^ ex)
			{
				DebugPrint(L"    Error 0x%x getting Contents folder\n", ex->HResult);
			}

		}).wait();

		return;
	}
	catch (Platform::Exception^ ex)
	{
		DebugPrint(L"    Error 0x%x\n", ex->HResult);
	}
}

void MainPage::LoadDLLFromPackage(Windows::ApplicationModel::Package^ package)
{
	Windows::Storage::StorageFolder^ Folder = package->InstalledLocation;
	auto asyncOp = Folder->GetFileAsync(L"OptionalPackageDLL.dll");

	Concurrency::create_task(asyncOp)
		.then([this, package](concurrency::task< Windows::Storage::StorageFile^ > task)
	{
		try
		{
			auto targetFile = task.get();

			if (targetFile->IsAvailable)
			{
				DebugPrint(L"    %ws is available:\n", targetFile->Name->Data());
				auto DllModule = LoadPackagedLibrary(targetFile->Name->Data(), 0);

				if (DllModule != NULL)
				{
					WriteToTextBox("Contains dll - loading code");
					try
					{
						auto procAddress = GetProcAddress(DllModule, "ExampleAPIExport");

						if (procAddress != NULL)
						{
							DebugPrint(L"        Value returned from ExampleAPIExport: %i\n", procAddress());
							auto ret = procAddress();
							WriteToTextBox(ret.ToString());							
						}
						else
						{
							DWORD error = GetLastError();
							error = error;
						}
					}
					catch (Platform::Exception^ ex)
					{
						DebugPrint(L"    Error getting address for ExampleAPIExport 0x%x\n", GetLastError());
					}
					FreeLibrary(DllModule);
				}
				else
				{
					HRESULT hr = HRESULT_FROM_WIN32(GetLastError());
					if (hr == 0x8007007e) //ERROR_MOD_NOT_FOUND
					{
						DebugPrint(L"    Could not load dll from OP - ERROR_MOD_NOT_FOUND. Make sure that your op is in a related set.\n");
						WriteToTextBox("Could not load dll from OP - ERROR_MOD_NOT_FOUND. Make sure that your op is in a related set.");
					}
					else
					{
						DebugPrint(L"    Error getting DLL file in package 0x%x\n", hr);
						WriteToTextBox("Error getting DLL file in package");
					}
				}
			}
		}
		catch (Platform::Exception^ ex)
		{
			if (ex->HResult == 0x80070002) //E_FILE_NOT_FOUND
			{
				DebugPrint(L"    No DLL to load in package\n");
			}
			else
			{
				DebugPrint(L"    Error 0x%x loading DLL file from OP package\n    %ws\n", ex->HResult, ex->Message->Data());
			}

		}
	}).wait();

	return;

}

void MainPage::WriteToTextBox(Platform::String^ str)
{
	auto dispatcher = Windows::ApplicationModel::Core::CoreApplication::MainView->Dispatcher->RunAsync(Windows::UI::Core::CoreDispatcherPriority::Normal,
		ref new Windows::UI::Core::DispatchedHandler([this, str]()
	{
		ApptextBox->Text += str +"\n";
	}));
}

void MyMainApp::MainPage::button_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	ReadOptionalPackageContent();
}


