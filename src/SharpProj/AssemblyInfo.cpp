#include "pch.h"

using namespace System;
using namespace System::Reflection;
using namespace System::Runtime::CompilerServices;
using namespace System::Runtime::InteropServices;
using namespace System::Security::Permissions;

#define LIT2(x) #x
#define LIT(x) LIT2(x)

#define PROJ_VERSION LIT(PROJ_VERSION_MAJOR) "." LIT(PROJ_VERSION_MINOR) "." LIT(PROJ_VERSION_PATCH)

[assembly:AssemblyTitleAttribute(L"SharpProj - Proj" PROJ_VERSION " wrapped for .Net 4+")];
[assembly:AssemblyDescriptionAttribute(L"")];
[assembly:AssemblyConfigurationAttribute(L"")];
[assembly:AssemblyCompanyAttribute(L"HP Inc.")];
[assembly:AssemblyProductAttribute(L"SharpProj")];
[assembly:AssemblyCopyrightAttribute(L"Copyright (c) HP Inc. 2021")];
[assembly:AssemblyTrademarkAttribute(L"")];
[assembly:AssemblyCultureAttribute(L"")];

[assembly:AssemblyVersionAttribute(PROJ_VERSION ".*")];

[assembly:ComVisible(false)];

[assembly:CLSCompliantAttribute(true)];
