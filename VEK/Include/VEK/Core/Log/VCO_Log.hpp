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

#pragma once

#include <VEK/Core/Container/VCO_String.hpp>
#include <VEK/Core/VCO_Console.hpp>

namespace VEK::Core {

  enum class KLogLevel: uint8_t {
    Info,
    Error,
    Trace,
    Debug,
    Warning
  };

  struct KLogEntry {
    KSafeString<> source;
    KSafeString<> message;

    KLogLevel level;
  };

  class KLogger {
    public:
      KLogger() = delete;
      
      static void Log(KSafeString<>& source, KSafeString<>& message, KLogLevel level = KLogLevel::Info);

    private:
      uint32_t m_LogCount = 0;
  };

}