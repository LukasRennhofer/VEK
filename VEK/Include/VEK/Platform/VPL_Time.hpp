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

#include <cstdint>

namespace VEK::Platform::Time {

    // Time units enumeration
    enum class ETimeUnit : uint8_t {
        Nanoseconds,
        Microseconds,
        Milliseconds,
        Seconds,
        Minutes,
        Hours,
        Days
    };

    // Time format enumeration
    enum class ETimeFormat : uint8_t {
        Unix,           // Unix timestamp
        ISO8601,        // ISO 8601 format
        Local,          // Local time format
        UTC             // UTC format
    };

    // Simple time duration struct
    struct Duration {
        uint64_t value;
        ETimeUnit unit;
        
        Duration(uint64_t val, ETimeUnit timeUnit) : value(val), unit(timeUnit) {}
        
        // Convert to milliseconds
        uint64_t ToMilliseconds() const {
            switch (unit) {
                case ETimeUnit::Nanoseconds:  return value / 1000000;
                case ETimeUnit::Microseconds: return value / 1000;
                case ETimeUnit::Milliseconds: return value;
                case ETimeUnit::Seconds:      return value * 1000;
                case ETimeUnit::Minutes:      return value * 60000;
                case ETimeUnit::Hours:        return value * 3600000;
                case ETimeUnit::Days:         return value * 86400000;
                default: return value;
            }
        }
        
        // Convert to microseconds
        uint64_t ToMicroseconds() const {
            switch (unit) {
                case ETimeUnit::Nanoseconds:  return value / 1000;
                case ETimeUnit::Microseconds: return value;
                case ETimeUnit::Milliseconds: return value * 1000;
                case ETimeUnit::Seconds:      return value * 1000000;
                case ETimeUnit::Minutes:      return value * 60000000;
                case ETimeUnit::Hours:        return value * 3600000000ULL;
                case ETimeUnit::Days:         return value * 86400000000ULL;
                default: return value;
            }
        }
        
        // Convert to seconds
        double ToSeconds() const {
            return ToMicroseconds() / 1000000.0;
        }
    };

    // Timer Class (needs an OS Instance to work)
    template<class TOSInstance>
    class Timer {
    private:
        TOSInstance* m_os;
        uint64_t m_startTime;
        
    public:
        Timer(TOSInstance* os) : m_os(os) { Reset(); }
        
        // Reset the timer to current time
        void Reset() {
            if (m_os) {
                m_startTime = m_os->GetTicks();
            }
        }
        
        // Get elapsed milliseconds since creation or last reset
        uint64_t GetElapsedMs() const {
            if (m_os) {
                return m_os->GetTicks() - m_startTime;
            }
            return 0;
        }
        
        // Get elapsed microseconds since creation or last reset
        uint64_t GetElapsedMicro() const {
            if (m_os) {
                uint64_t currentMicro = m_os->GetTicksMicro();
                uint64_t startMicro = m_startTime * 1000; // Convert start time from ms to microseconds
                return currentMicro - startMicro;
            }
            return 0;
        }
        
        // Get elapsed nanoseconds since creation or last reset
        uint64_t GetElapsedNano() const {
            if (m_os) {
                return m_os->GetTicksNano() - (m_startTime * 1000000); // Convert start time to nanoseconds
            }
            return 0;
        }
        
        // Get elapsed seconds as floating point
        double GetElapsedSeconds() const {
            return GetElapsedMs() / 1000.0;
        }
        
        // Get elapsed time as Duration
        Duration GetElapsed(ETimeUnit unit = ETimeUnit::Milliseconds) const {
            switch (unit) {
                case ETimeUnit::Nanoseconds:  return Duration(GetElapsedNano(), unit);
                case ETimeUnit::Microseconds: return Duration(GetElapsedMicro(), unit);
                case ETimeUnit::Milliseconds: return Duration(GetElapsedMs(), unit);
                case ETimeUnit::Seconds:      return Duration(GetElapsedMs() / 1000, unit);
                default: return Duration(GetElapsedMs(), ETimeUnit::Milliseconds);
            }
        }
    };

    // Time conversion utilities
    namespace Convert {
        constexpr uint64_t MillisecondsToMicroseconds(uint64_t ms) { return ms * 1000; }
        constexpr uint64_t MillisecondsToNanoseconds(uint64_t ms) { return ms * 1000000; }
        constexpr uint64_t MicrosecondsToMilliseconds(uint64_t us) { return us / 1000; }
        constexpr uint64_t MicrosecondsToNanoseconds(uint64_t us) { return us * 1000; }
        constexpr uint64_t NanosecondsToMicroseconds(uint64_t ns) { return ns / 1000; }
        constexpr uint64_t NanosecondsToMilliseconds(uint64_t ns) { return ns / 1000000; }
        constexpr double MillisecondsToSeconds(uint64_t ms) { return ms / 1000.0; }
        constexpr uint64_t SecondsToMilliseconds(double s) { return static_cast<uint64_t>(s * 1000); }
    }

}
