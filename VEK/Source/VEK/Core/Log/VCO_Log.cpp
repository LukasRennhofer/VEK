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

namespace VEK::Core {

    // Define static members
    uint32_t KLogger::m_LogCount = 0;
    KVector<KLogEntry> KLogger::m_Entries;

    const KSafeString<> KLogger::LevelToString(KLogLevel level) {
      switch (level) {
        case KLogLevel::Info:
          return KSafeString<>("Info");
        case KLogLevel::Debug:
          return KSafeString<>("Debug");
        case KLogLevel::Error:
          return KSafeString<>("Error");
        case KLogLevel::Trace:
          return KSafeString<>("Trace");
        case KLogLevel::Warning:
          return KSafeString<>("Warning");
        
      }
    }

    void KLogger::Log(const KSafeString<>& source, const KSafeString<>& message, KLogLevel level) {
        KLogEntry entry;

        entry.source  = source;
        entry.message = message;
        entry.level   = level;

        m_Entries.push_back(entry);

        m_LogCount += 1;

        // TODO: Make it choose, to ouput on console or not

        KSafeString<> new_string;

        new_string += "[";
        new_string += LevelToString(level).c_str();
        new_string += "] ";
        new_string += "[";
        new_string += source.c_str();
        new_string += "] ";
        new_string += message.c_str();

        
    }
}