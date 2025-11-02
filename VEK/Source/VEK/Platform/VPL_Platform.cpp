/*
================================================================================
  VEK (Vantor Engine Kernel) - Used by Vantor Studios
--------------------------------------------------------------------------------
  Author  : Lukas Rennhofer (lukas.renn@aon.at)
  License : GNU General Public License v3.0

  “Order the chaos, frame the void — and call it a world.”
================================================================================
*/

#include <VEK/Platform/VPL_Platform.hpp>

#ifdef VEK_WINDOWS
#include <VEK/Platform/Impl/Windows/VPL_WindowsOS.hpp>
#endif

#ifdef VEK_LINUX
#include <VEK/Platform/Impl/Linux/VPL_LinuxOS.hpp>
#endif

#include <iostream>

namespace VEK::Platform {

    std::unique_ptr<IOS> IOS::Create() {
#ifdef VEK_WINDOWS
        return std::make_unique<WindowsOS>();
#elif defined(VEK_LINUX)
        return std::make_unique<LinuxOS>();
#else
        return nullptr;
#endif
    }

}