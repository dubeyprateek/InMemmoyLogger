#pragma once
#include "Class.g.h"

namespace winrt::TestDll::implementation
{
    struct Class : ClassT<Class>
    {
        Class() = default;

        int32_t MyProperty();
        void MyProperty(int32_t value);
        void TestLogger();
    };
}
namespace winrt::TestDll::factory_implementation
{
    struct Class : ClassT<Class, implementation::Class>
    {
    };
}
