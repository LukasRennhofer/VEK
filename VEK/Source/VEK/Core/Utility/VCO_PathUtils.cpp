/*
================================================================================
  VEK (Vantor Engine Kernel) - Used by Vantor Studios
--------------------------------------------------------------------------------
  Author  : Lukas Rennhofer (lukas.renn@aon.at)
  License : GNU General Public License v3.0

  “Order the chaos, frame the void — and call it a world.”
================================================================================
*/

#include <VEK/Core/Utility/VCO_PathUtils.hpp>
#include <algorithm>
#include <cctype>

namespace VEK::Core {

    KSafeString<> KPathUtils::CombinePath(const KSafeString<>& path1, const KSafeString<>& path2) {
        if (path1.empty()) return path2;
        if (path2.empty()) return path1;

        char separator = DetectPathSeparator(path1);
        if (separator == '\0') separator = '/'; // Default to Unix separator

        KSafeString<> result = path1;
        if (result.back() != separator && result.back() != '\\' && result.back() != '/') {
            result += separator;
        }
        
        // Remove leading separators from path2
        KSafeString<> cleanPath2 = path2;
        while (!cleanPath2.empty() && (cleanPath2[0] == '/' || cleanPath2[0] == '\\')) {
            cleanPath2 = cleanPath2.substr(1);
        }
        
        result += cleanPath2;
        return result;
    }

    KSafeString<> KPathUtils::CombinePath(const KSafeString<>& path1, const KSafeString<>& path2, const KSafeString<>& path3) {
        return CombinePath(CombinePath(path1, path2), path3);
    }

    KSafeString<> KPathUtils::GetFileExtension(const KSafeString<>& path) {
        size_t dotPos = path.find_last_of('.');
        size_t slashPos = path.find_last_of("/\\");
        
        if (dotPos != KSafeString<>::npos && 
            (slashPos == KSafeString<>::npos || dotPos > slashPos)) {
            return path.substr(dotPos);
        }
        return KSafeString<>();
    }

    KSafeString<> KPathUtils::GetFileName(const KSafeString<>& path) {
        size_t slashPos = path.find_last_of("/\\");
        if (slashPos != KSafeString<>::npos) {
            return path.substr(slashPos + 1);
        }
        return path;
    }

    KSafeString<> KPathUtils::GetFileNameWithoutExtension(const KSafeString<>& path) {
        KSafeString<> filename = GetFileName(path);
        size_t dotPos = filename.find_last_of('.');
        if (dotPos != KSafeString<>::npos) {
            return filename.substr(0, dotPos);
        }
        return filename;
    }

    KSafeString<> KPathUtils::GetDirectoryName(const KSafeString<>& path) {
        size_t slashPos = path.find_last_of("/\\");
        if (slashPos != KSafeString<>::npos) {
            return path.substr(0, slashPos);
        }
        return KSafeString<>();
    }

    KSafeString<> KPathUtils::NormalizePath(const KSafeString<>& path) {
        char separator = DetectPathSeparator(path);
        if (separator == '\0') separator = '/';
        return NormalizePath(path, separator);
    }

    KSafeString<> KPathUtils::NormalizePath(const KSafeString<>& path, char pathSeparator) {
        if (path.empty()) return path;

        KSafeString<> result = path;
        
        // Replace all separators with the target separator
        for (size_t i = 0; i < result.size(); ++i) {
            if (result[i] == '/' || result[i] == '\\') {
                result[i] = pathSeparator;
            }
        }

        // Remove duplicate separators
        size_t writePos = 0;
        bool lastWasSeparator = false;
        for (size_t readPos = 0; readPos < result.size(); ++readPos) {
            if (result[readPos] == pathSeparator) {
                if (!lastWasSeparator) {
                    result[writePos++] = pathSeparator;
                    lastWasSeparator = true;
                }
            } else {
                result[writePos++] = result[readPos];
                lastWasSeparator = false;
            }
        }
        result.resize(writePos);

        return RemoveTrailingSeparators(result, pathSeparator);
    }

    bool KPathUtils::IsAbsolutePath(const KSafeString<>& path) {
        if (path.empty()) return false;
        
        // Unix absolute path
        if (path[0] == '/') return true;
        
        // Windows absolute path (C:\, D:\, etc.)
        if (path.size() >= 3 && std::isalpha(path[0]) && path[1] == ':' && 
            (path[2] == '\\' || path[2] == '/')) {
            return true;
        }
        
        // UNC path (\\server\share)
        if (path.size() >= 2 && path[0] == '\\' && path[1] == '\\') {
            return true;
        }
        
        return false;
    }

    bool KPathUtils::IsRelativePath(const KSafeString<>& path) {
        return !IsAbsolutePath(path);
    }

    bool KPathUtils::HasExtension(const KSafeString<>& path) {
        return !GetFileExtension(path).empty();
    }

    bool KPathUtils::HasExtension(const KSafeString<>& path, const KSafeString<>& extension) {
        KSafeString<> pathExt = GetFileExtension(path);
        KSafeString<> targetExt = extension;
        
        // Ensure extension starts with a dot
        if (!targetExt.empty() && targetExt[0] != '.') {
            targetExt = "." + targetExt;
        }
        
        // Case-insensitive comparison
        if (pathExt.size() != targetExt.size()) return false;
        
        for (size_t i = 0; i < pathExt.size(); ++i) {
            if (std::tolower(pathExt[i]) != std::tolower(targetExt[i])) {
                return false;
            }
        }
        
        return true;
    }

    KSafeString<> KPathUtils::ToUnixPath(const KSafeString<>& path) {
        return NormalizePath(path, '/');
    }

    KSafeString<> KPathUtils::ToWindowsPath(const KSafeString<>& path) {
        return NormalizePath(path, '\\');
    }

    KSafeString<> KPathUtils::ChangeExtension(const KSafeString<>& path, const KSafeString<>& newExtension) {
        KSafeString<> pathWithoutExt = GetDirectoryName(path);
        if (!pathWithoutExt.empty()) {
            pathWithoutExt = CombinePath(pathWithoutExt, GetFileNameWithoutExtension(path));
        } else {
            pathWithoutExt = GetFileNameWithoutExtension(path);
        }
        
        if (!newExtension.empty()) {
            if (newExtension[0] != '.') {
                pathWithoutExt += ".";
            }
            pathWithoutExt += newExtension;
        }
        
        return pathWithoutExt;
    }

    bool KPathUtils::IsValidPath(const KSafeString<>& path) {
        if (path.empty()) return false;
        
        // Check for invalid characters (basic check)
        const char* invalidChars = "<>:\"|?*";
        for (size_t i = 0; i < path.size(); ++i) {
            char c = path[i];
            if (c < 32) return false; // Control characters
            
            for (const char* invalid = invalidChars; *invalid; ++invalid) {
                if (c == *invalid) return false;
            }
        }
        
        return true;
    }

    bool KPathUtils::IsValidFileName(const KSafeString<>& filename) {
        if (filename.empty()) return false;
        if (filename == "." || filename == "..") return false;
        
        // Check for path separators in filename
        if (filename.find('/') != KSafeString<>::npos || 
            filename.find('\\') != KSafeString<>::npos) {
            return false;
        }
        
        return IsValidPath(filename);
    }

    char KPathUtils::DetectPathSeparator(const KSafeString<>& path) {
        size_t backslashCount = 0;
        size_t forwardSlashCount = 0;
        
        for (char c : path) {
            if (c == '\\') backslashCount++;
            else if (c == '/') forwardSlashCount++;
        }
        
        if (backslashCount > forwardSlashCount) return '\\';
        if (forwardSlashCount > 0) return '/';
        
        return '\0'; // No separators found
    }

    KSafeString<> KPathUtils::RemoveTrailingSeparators(const KSafeString<>& path, char separator) {
        if (path.empty()) return path;
        
        KSafeString<> result = path;
        while (!result.empty() && result.back() == separator) {
            result.pop_back();
        }
        
        // Don't remove the root separator
        if (result.empty() && path[0] == separator) {
            result = separator;
        }
        
        return result;
    }

} // namespace VEK::Core
