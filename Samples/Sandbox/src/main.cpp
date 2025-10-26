/*
================================================================================
  VEK (Vantor Engine Kernel)
--------------------------------------------------------------------------------
  Author  : Lukas Rennhofer (lukas.renn@aon.at)
  License : GNU General Public License v3.0

  "God made the world. I just made a computer kingdom in it."
      — Terry A. Davis
================================================================================
*/

#include <VEK/Core/Log/VCO_Log.hpp>

#include <iostream>

int main(int argc, char** argv) {

    VEK::Core::KLogger::Log("TestEntry", "This is a test!", VEK::Core::KLogLevel::Debug);

    std::cout << VEK::Core::KLogger::GetLogCount() << std::endl;

    auto entry = VEK::Core::KLogger::GetLogEntry(0);

    std::cout << "[" << entry.source.c_str() << "] " << entry.message.c_str() << std::endl;

    return 0;
}