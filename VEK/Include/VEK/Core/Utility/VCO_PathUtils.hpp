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

#include <cstdint>

namespace VEK::Core {

    /**
     * @brief Platform-agnostic path manipulation utilities
     * 
     * This class provides cross-platform path manipulation functions
     * that work regardless of the underlying platform's path conventions.
     */
    class KPathUtils {
    public:
        KPathUtils() = delete;

        // Path manipulation
        static KSafeString<> CombinePath(const KSafeString<>& path1, const KSafeString<>& path2);
        static KSafeString<> CombinePath(const KSafeString<>& path1, const KSafeString<>& path2, const KSafeString<>& path3);
        static KSafeString<> GetFileExtension(const KSafeString<>& path);
        static KSafeString<> GetFileName(const KSafeString<>& path);
        static KSafeString<> GetFileNameWithoutExtension(const KSafeString<>& path);
        static KSafeString<> GetDirectoryName(const KSafeString<>& path);
        
        // Path normalization
        static KSafeString<> NormalizePath(const KSafeString<>& path);
        static KSafeString<> NormalizePath(const KSafeString<>& path, char pathSeparator);
        
        // Path queries
        static bool IsAbsolutePath(const KSafeString<>& path);
        static bool IsRelativePath(const KSafeString<>& path);
        static bool HasExtension(const KSafeString<>& path);
        static bool HasExtension(const KSafeString<>& path, const KSafeString<>& extension);
        
        // Path conversion
        static KSafeString<> ToUnixPath(const KSafeString<>& path);
        static KSafeString<> ToWindowsPath(const KSafeString<>& path);
        static KSafeString<> ChangeExtension(const KSafeString<>& path, const KSafeString<>& newExtension);
        
        // Path validation
        static bool IsValidPath(const KSafeString<>& path);
        static bool IsValidFileName(const KSafeString<>& filename);
        
    private:
        // Helper functions
        static char DetectPathSeparator(const KSafeString<>& path);
        static KSafeString<> RemoveTrailingSeparators(const KSafeString<>& path, char separator);
    };

} // namespace VEK::Core
