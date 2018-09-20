#include "pch.h"
#include "OptionalPackageDLL.h"

// Optional Package Code
EXTERN_C __declspec(dllexport)  int __stdcall ExampleAPIExport()
{
    return 40;
}
