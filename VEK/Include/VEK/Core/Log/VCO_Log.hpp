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
#include <VEK/Core/Container/VCO_Vector.hpp>

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
      
      static void Log(const KSafeString<>& source, const KSafeString<>& message, KLogLevel level = KLogLevel::Info);

      static uint32_t GetLogCount() { return m_LogCount; }

      // TODO: Write safer function
      static KLogEntry GetLogEntry(uint32_t index) { if (!(index > m_LogCount - 1)) { return m_Entries[index]; } }
      static const KSafeString<> LevelToString(KLogLevel level);

    private:
      static uint32_t m_LogCount;
      static KVector<KLogEntry> m_Entries; 
  };

}