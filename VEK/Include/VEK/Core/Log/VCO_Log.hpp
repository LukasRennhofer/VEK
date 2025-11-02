/*
================================================================================
  VEK (Vantor Engine Kernel) - Used by Vantor Studios
--------------------------------------------------------------------------------
  Author  : Lukas Rennhofer (lukas.renn@aon.at)
  License : GNU General Public License v3.0

  “Order the chaos, frame the void — and call it a world.”
================================================================================
*/

#pragma once

#include <VEK/Core/Container/VCO_String.hpp>
#include <VEK/Core/Container/VCO_Vector.hpp>

#include <VEK/Core/VCO_Console.hpp>

// Configuration macros
#ifndef VEK_LOGGING_ENABLED
    #define VEK_LOGGING_ENABLED 1
#endif

#ifndef VEK_LOG_TO_CONSOLE
    #define VEK_LOG_TO_CONSOLE 1
#endif

namespace VEK::Core {

  enum class KLogLevel: uint8_t {
    Info = 0,
    Debug = 1,
    Warning = 2,
    Error = 3,
    Trace = 4
  };

  struct KLogEntry {
    KSafeString<> source;
    KSafeString<> message;
    KLogLevel level;
    // TODO: Add timestamp here: uint64_t timestamp;
  };

  class KLogger {
    public:
      KLogger() = delete;
      
      // Main logging function
      static void Log(const KSafeString<>& source, const KSafeString<>& message, KLogLevel level = KLogLevel::Info);
      
      // Convenience functions for different log levels
      static void Info(const KSafeString<>& source, const KSafeString<>& message);
      static void Debug(const KSafeString<>& source, const KSafeString<>& message);
      static void Warning(const KSafeString<>& source, const KSafeString<>& message);
      static void Error(const KSafeString<>& source, const KSafeString<>& message);
      static void Trace(const KSafeString<>& source, const KSafeString<>& message);

      // Log entry management
      static uint32_t GetLogCount() { return m_LogCount; }
      static KLogEntry GetLogEntry(uint32_t index);
      static void ClearLogs();
      
      // Configuration
      static void SetConsoleOutput(bool enabled) { m_ConsoleOutput = enabled; }
      static bool IsConsoleOutputEnabled() { return m_ConsoleOutput; }
      
      static void SetEnabled(bool enabled) { m_Enabled = enabled; }
      static bool IsEnabled() { return m_Enabled; }
      
      static void SetLogLevel(KLogLevel minLevel) { m_MinLogLevel = minLevel; }
      static KLogLevel GetLogLevel() { return m_MinLogLevel; }

      // Utility functions
      static const KSafeString<> LevelToString(KLogLevel level);
      static KConsoleColor LevelToColor(KLogLevel level);

    private:
      static void OutputToConsole(const KSafeString<>& formattedMessage, KLogLevel level);

    private:
      static uint32_t m_LogCount;
      static KVector<KLogEntry> m_Entries; 
      static bool m_ConsoleOutput;
      static bool m_Enabled;
      static KLogLevel m_MinLogLevel;
  };
}

// Macros
#define VEK_LOG_INFO(source, message)    VEK::Core::KLogger::Info(source, message)
#define VEK_LOG_DEBUG(source, message)   VEK::Core::KLogger::Debug(source, message)
#define VEK_LOG_WARNING(source, message) VEK::Core::KLogger::Warning(source, message)
#define VEK_LOG_ERROR(source, message)   VEK::Core::KLogger::Error(source, message)
#define VEK_LOG_TRACE(source, message)   VEK::Core::KLogger::Trace(source, message)