/*
================================================================================
  VEK (Vantor Engine Kernel) - Used by Vantor Studios
--------------------------------------------------------------------------------
  Author  : Lukas Rennhofer (lukas.renn@aon.at)
  License : GNU General Public License v3.0

  “Order the chaos, frame the void — and call it a world.”
================================================================================
*/

#include <VEK/Core/Log/VCO_Log.hpp>

namespace VEK::Core {

    // Define static members
    uint32_t KLogger::m_LogCount = 0;
    KVector<KLogEntry> KLogger::m_Entries;
    bool KLogger::m_ConsoleOutput = true;
    bool KLogger::m_Enabled = true;
    KLogLevel KLogger::m_MinLogLevel = KLogLevel::Info;

    const KSafeString<> KLogger::LevelToString(KLogLevel level) {
        switch (level) {
            case KLogLevel::Info:
                return KSafeString<>("INFO");
            case KLogLevel::Debug:
                return KSafeString<>("DEBUG");
            case KLogLevel::Warning:
                return KSafeString<>("WARNING");
            case KLogLevel::Error:
                return KSafeString<>("ERROR");
            case KLogLevel::Trace:
                return KSafeString<>("TRACE");
            default:
                return KSafeString<>("UNKNOWN");
        }
    }

    KConsoleColor KLogger::LevelToColor(KLogLevel level) {
        switch (level) {
            case KLogLevel::Info:
                return KConsoleColor::White;
            case KLogLevel::Debug:
                return KConsoleColor::Cyan;
            case KLogLevel::Warning:
                return KConsoleColor::Yellow;
            case KLogLevel::Error:
                return KConsoleColor::Red;
            case KLogLevel::Trace:
                return KConsoleColor::Magenta;
            default:
                return KConsoleColor::Default;
        }
    }

    void KLogger::Log(const KSafeString<>& source, const KSafeString<>& message, KLogLevel level) {
        #if VEK_LOGGING_ENABLED
            if (!m_Enabled) return;
            
            // Check if this log level should be processed
            if (static_cast<uint8_t>(level) < static_cast<uint8_t>(m_MinLogLevel)) {
                return;
            }

            // Create log entry
            KLogEntry entry;
            entry.source = source;
            entry.message = message;
            entry.level = level;

            // Store in memory (if we want to keep logs)
            m_Entries.push_back(entry);
            m_LogCount++;

            // Format message for console output
            KSafeString<> formattedMessage;
            formattedMessage += "[";
            formattedMessage += LevelToString(level).c_str();
            formattedMessage += "] [";
            formattedMessage += source.c_str();
            formattedMessage += "] ";
            formattedMessage += message.c_str();

            // Output to console if enabled
            if (m_ConsoleOutput) {
                OutputToConsole(formattedMessage, level);
            }
        #endif
    }

    void KLogger::Info(const KSafeString<>& source, const KSafeString<>& message) {
        Log(source, message, KLogLevel::Info);
    }

    void KLogger::Debug(const KSafeString<>& source, const KSafeString<>& message) {
        Log(source, message, KLogLevel::Debug);
    }

    void KLogger::Warning(const KSafeString<>& source, const KSafeString<>& message) {
        Log(source, message, KLogLevel::Warning);
    }

    void KLogger::Error(const KSafeString<>& source, const KSafeString<>& message) {
        Log(source, message, KLogLevel::Error);
    }

    void KLogger::Trace(const KSafeString<>& source, const KSafeString<>& message) {
        Log(source, message, KLogLevel::Trace);
    }

    KLogEntry KLogger::GetLogEntry(uint32_t index) {
        if (index < m_LogCount && index < m_Entries.size()) {
            return m_Entries[index];
        }
        
        // Return empty entry if index is invalid
        KLogEntry empty;
        empty.source = KSafeString<>("INVALID");
        empty.message = KSafeString<>("Invalid log entry index");
        empty.level = KLogLevel::Error;
        return empty;
    }

    void KLogger::ClearLogs() {
        m_Entries.clear();
        m_LogCount = 0;
    }

    void KLogger::OutputToConsole(const KSafeString<>& formattedMessage, KLogLevel level) {
        #if VEK_LOG_TO_CONSOLE
            KConsoleColor color = LevelToColor(level);
            
            #ifdef VEK_NSX
                // For Nintendo Switch, we might want to use different output methods
                KConsoleStream::WriteLine(formattedMessage, color);
            #else
                // Standard console output for other platforms
                KConsoleStream::WriteLine(formattedMessage, color);
            #endif
        #endif
    }
}