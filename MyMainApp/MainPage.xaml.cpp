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

void MainPage::ReadOptionalPackageContentAsync()
{
    concurrency::create_task([this]()
    {
        WriteToTextBox("Enumerating Packages");
        auto optionalPackages = GetOptionalPackages();
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

std::vector<Windows::ApplicationModel::Package^> MainPage::GetOptionalPackages()
{
    DebugPrint(L"    Searching for optional packages...\n");

    // The dependencies list is where the list of optional packages (OP) can be determined
    std::vector<Windows::ApplicationModel::Package^> optionalPackages;

    for (auto package : Windows::ApplicationModel::Package::Current->Dependencies)
    {
        if (package->IsOptional)
        {
            DebugPrint(L"    Optional Package found - %ws\n", package->Id->FullName->Data());
            optionalPackages.push_back(package);
        }
    }

    return optionalPackages;
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
    concurrency::create_task(package->InstalledLocation->TryGetItemAsync(LR"(Content\SampleFile.txt)"))
        .then([this](concurrency::task<Windows::Storage::IStorageItem^> result)
    {
        if (result.get())
        {
            auto sampleFile = safe_cast<Windows::Storage::StorageFile^>(result.get());
            if (sampleFile->IsAvailable)
            {
                WriteToTextBox("Found SampleFile.txt - loading contents");
                DebugPrint(L"    %ws is available:\n", sampleFile->Name->Data());
                concurrency::create_task(sampleFile->OpenAsync(Windows::Storage::FileAccessMode::Read))
                    .then([this, sampleFile](concurrency::task<Windows::Storage::Streams::IRandomAccessStream^> task)
                {
                    auto readStream = task.get();
                    UINT64 const size = readStream->Size;
                    if (size <= MAXUINT32)
                    {
                        auto dataReader = ref new Windows::Storage::Streams::DataReader(readStream);
                        concurrency::create_task(dataReader->LoadAsync(static_cast<UINT32>(size)))
                            .then([this, sampleFile, dataReader](unsigned int numBytesLoaded)
                        {
                            auto fileContent = dataReader->ReadString(numBytesLoaded);

                            WriteToTextBox(fileContent);

                            delete dataReader; // As a best practice, explicitly close the dataReader resource as soon as it is no longer needed.
                            DebugPrint(L"        %ws\n", fileContent->Data());
                        });
                    }
                }).wait();
            }
        }
        else
        {
            DebugPrint(L"    SampleFile.txt not available\n");
        }
    }).wait();
}

void MainPage::LoadDLLFromPackage(Windows::ApplicationModel::Package^ package)
{
    concurrency::create_task(package->InstalledLocation->TryGetItemAsync(L"OptionalPackageDLL.dll"))
        .then([this, package](concurrency::task<Windows::Storage::IStorageItem^> task)
    {
        if (task.get())
        {
            auto targetFile = safe_cast<Windows::Storage::StorageFile^>(task.get());
            if (targetFile)
            {
                if (targetFile->IsAvailable)
                {
                    DebugPrint(L"    %ws is available:\n", targetFile->Name->Data());
                    auto name = targetFile->Name->Data();
                    auto dllModule = LoadPackagedLibrary(name, 0);
                    if (dllModule)
                    {
                        WriteToTextBox("Contains dll - loading code");

                        auto procAddress = GetProcAddress(dllModule, "ExampleAPIExport");
                        if (procAddress)
                        {
                            DebugPrint(L"        Value returned from ExampleAPIExport: %i\n", procAddress());
                            auto ret = procAddress();
                            WriteToTextBox(ret.ToString());
                        }
                        FreeLibrary(dllModule);
                    }
                    else
                    {
                        const DWORD error = GetLastError();
                        DebugPrint(L"        LoadPackagedLibrary failed, make sure package certificates are configured: %i\n", error);
                    }
                }
            }
            else
            {
                DebugPrint(L"    Could not load dll from the optional package, make sure it is installed.\n");
                WriteToTextBox("Could not load dll from the optional package, make sure it is installed.");
            }
        }
    }).wait();
}

void MainPage::WriteToTextBox(Platform::String^ str)
{
    auto dispatcher = Windows::ApplicationModel::Core::CoreApplication::MainView->Dispatcher->RunAsync(Windows::UI::Core::CoreDispatcherPriority::Normal,
        ref new Windows::UI::Core::DispatchedHandler([this, str]()
    {
        ApptextBox->Text += str + "\n";
    }));
}

void MyMainApp::MainPage::button_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
    ReadOptionalPackageContentAsync();
}


