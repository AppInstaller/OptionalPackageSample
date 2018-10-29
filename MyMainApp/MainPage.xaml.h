//
// MainPage.xaml.h
// Declaration of the MainPage class.
//

#pragma once

#include "MainPage.g.h"

namespace MyMainApp
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public ref class MainPage sealed
    {
    public:
        MainPage();
    private:
        Windows::ApplicationModel::PackageCatalog^ catalog;
        std::vector<Windows::ApplicationModel::Package^> GetOptionalPackages();
        void MainPage::OnPackageInstalling(Windows::ApplicationModel::PackageCatalog ^sender, Windows::ApplicationModel::PackageInstallingEventArgs ^args);
        void MainPage::HookupCatalog();
        void LoadTextFromPackage(Windows::ApplicationModel::Package^ package);
        void LoadDLLFromPackage(Windows::ApplicationModel::Package^ package);
        void MainPage::WriteToTextBox(Platform::String^ str);
        void MainPage::ReadOptionalPackageContentAsync();

        void DebugPrint(const wchar_t* msg, ...)
        {
            va_list ap;
            va_start(ap, msg);

            int len = _vscwprintf(msg, ap) + 1;
            wchar_t * buf = (wchar_t*)malloc(len * sizeof(wchar_t));
            vswprintf_s(buf, len, msg, ap);
            buf[len - 1] = 0;

            va_end(ap);

            OutputDebugString(buf);

            free(buf);
        };
        void button_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
    };
}
