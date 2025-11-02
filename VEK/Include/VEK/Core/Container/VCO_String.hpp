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

#include <algorithm>
#include <cassert>
#include <cstring>
#include <cstddef>
#include <cstdint>
#include <memory>

namespace VEK::Core {

    // Internal Implementation for a KSafeString
    template <typename Alloc = std::allocator<char>> class KSafeString
    {
        public:
            static constexpr size_t SSO_THRESHOLD = 23;
            static constexpr size_t npos = static_cast<size_t>(-1);

            KSafeString() noexcept : m_size(0), m_using_sso(true) { m_sso_data[0] = '\0'; }

            KSafeString(const char *str) : KSafeString() { assign(str); }

            // Copy constructor
            KSafeString(const KSafeString& other) : KSafeString() {
                assign(other.c_str());
            }

            // Move constructor
            KSafeString(KSafeString&& other) noexcept 
                : m_size(other.m_size), m_capacity(other.m_capacity), m_using_sso(other.m_using_sso), m_allocator(std::move(other.m_allocator))
            {
                if (m_using_sso) {
                    std::memcpy(m_sso_data, other.m_sso_data, m_size + 1);
                } else {
                    m_data = other.m_data;
                    other.m_data = nullptr;
                }
                
                // Reset other to valid state
                other.m_size = 0;
                other.m_capacity = 0;
                other.m_using_sso = true;
                other.m_sso_data[0] = '\0';
            }

            // Assignment operator
            KSafeString& operator=(const KSafeString& other) {
                if (this != &other) {
                    assign(other.c_str());
                }
                return *this;
            }

            // Move assignment operator
            KSafeString& operator=(KSafeString&& other) noexcept {
                if (this != &other) {
                    // Clean up current allocation
                    if (!m_using_sso) {
                        m_allocator.deallocate(m_data, m_capacity + 1);
                    }
                    
                    // Move from other
                    m_size = other.m_size;
                    m_capacity = other.m_capacity;
                    m_using_sso = other.m_using_sso;
                    m_allocator = std::move(other.m_allocator);
                    
                    if (m_using_sso) {
                        std::memcpy(m_sso_data, other.m_sso_data, m_size + 1);
                    } else {
                        m_data = other.m_data;
                        other.m_data = nullptr;
                    }
                    
                    // Reset other to valid state
                    other.m_size = 0;
                    other.m_capacity = 0;
                    other.m_using_sso = true;
                    other.m_sso_data[0] = '\0';
                }
                return *this;
            }

            KSafeString& operator=(char c) {
                clear();
                push_back(c);
                return *this;
            }

            ~KSafeString()
            {
                if (!m_using_sso)
                {
                    m_allocator.deallocate(m_data, m_capacity + 1);
                }
            }

            void assign(const char *str)
            {
                // Clean up existing allocation if not using SSO
                if (!m_using_sso)
                {
                    m_allocator.deallocate(m_data, m_capacity + 1);
                    m_using_sso = true; // Reset to SSO state
                }

                size_t len = std::strlen(str);
                if (len <= SSO_THRESHOLD)
                {
                    std::memcpy(m_sso_data, str, len + 1);
                    m_size      = len;
                    m_using_sso = true;
                }
                else
                {
                    m_data = m_allocator.allocate(len + 1);
                    std::memcpy(m_data, str, len + 1);
                    m_size      = len;
                    m_capacity  = len;
                    m_using_sso = false;
                }
            }

            size_t      size() const { return m_size; }
            bool        empty() const { return m_size == 0; }
            const char *c_str() const { return m_using_sso ? m_sso_data : m_data; }

            // String operations
            size_t find(const char* substr) const
            {
                const char* found = std::strstr(c_str(), substr);
                return found ? static_cast<size_t>(found - c_str()) : npos;
            }

            size_t find(const char* substr, size_t pos) const
            {
                if (pos >= m_size) return npos;
                const char* found = std::strstr(c_str() + pos, substr);
                return found ? static_cast<size_t>(found - c_str()) : npos;
            }

            size_t find(char c) const
            {
                const char* found = std::strchr(c_str(), c);
                return found ? static_cast<size_t>(found - c_str()) : npos;
            }

            size_t find(const KSafeString& str) const
            {
                return find(str.c_str());
            }

            size_t find_last_of(const char* chars) const
            {
                const char* str = c_str();
                for (size_t i = m_size; i > 0; --i) {
                    size_t idx = i - 1;
                    if (std::strchr(chars, str[idx]) != nullptr) {
                        return idx;
                    }
                }
                return npos;
            }

            size_t find_last_of(char c) const
            {
                const char* str = c_str();
                for (size_t i = m_size; i > 0; --i) {
                    size_t idx = i - 1;
                    if (str[idx] == c) {
                        return idx;
                    }
                }
                return npos;
            }

            KSafeString substr(size_t pos) const
            {
                if (pos >= m_size) return KSafeString();
                return substr(pos, m_size - pos);
            }

            KSafeString substr(size_t pos, size_t len) const
            {
                if (pos >= m_size) return KSafeString();
                len = std::min(len, m_size - pos);
                
                char* temp = new char[len + 1];
                std::memcpy(temp, c_str() + pos, len);
                temp[len] = '\0';
                KSafeString result(temp);
                delete[] temp;
                return result;
            }

            // Comparison operators
            bool operator==(const char* str) const
            {
                return std::strcmp(c_str(), str) == 0;
            }

            bool operator==(const KSafeString& other) const
            {
                return m_size == other.m_size && std::strcmp(c_str(), other.c_str()) == 0;
            }

            // Access last character
            char back() const
            {
                assert(m_size > 0);
                return m_using_sso ? m_sso_data[m_size - 1] : m_data[m_size - 1];
            }

            char& back()
            {
                assert(m_size > 0);
                return m_using_sso ? m_sso_data[m_size - 1] : m_data[m_size - 1];
            }

            // Remove last character
            void pop_back()
            {
                if (m_size > 0) {
                    --m_size;
                    if (m_using_sso) {
                        m_sso_data[m_size] = '\0';
                    } else {
                        m_data[m_size] = '\0';
                    }
                }
            }

            // Resize string
            void resize(size_t new_size, char fill_char = '\0')
            {
                if (new_size < m_size) {
                    // Shrink
                    m_size = new_size;
                    if (m_using_sso) {
                        m_sso_data[m_size] = '\0';
                    } else {
                        m_data[m_size] = '\0';
                    }
                } else if (new_size > m_size) {
                    // Grow
                    if (new_size <= SSO_THRESHOLD && m_using_sso) {
                        // Can still use SSO
                        std::memset(m_sso_data + m_size, fill_char, new_size - m_size);
                        m_size = new_size;
                        m_sso_data[m_size] = '\0';
                    } else {
                        // Need heap allocation or grow existing heap
                        char* new_data = m_allocator.allocate(new_size + 1);
                        std::memcpy(new_data, c_str(), m_size);
                        std::memset(new_data + m_size, fill_char, new_size - m_size);
                        new_data[new_size] = '\0';
                        
                        if (!m_using_sso) {
                            m_allocator.deallocate(m_data, m_capacity + 1);
                        }
                        
                        m_data = new_data;
                        m_capacity = new_size;
                        m_size = new_size;
                        m_using_sso = false;
                    }
                }
            }

            uint32_t hash() const noexcept
            {
                constexpr uint32_t FNV_offset_basis = 2166136261u;
                constexpr uint32_t FNV_prime = 16777619u;

                uint32_t hash = FNV_offset_basis;
                const char* ptr = c_str();
                for (size_t i = 0; i < m_size; ++i)
                {
                    hash ^= static_cast<uint8_t>(ptr[i]);
                    hash *= FNV_prime;
                }
                return hash;
            }

            void clear() {
                if (!m_using_sso) {
                    m_allocator.deallocate(m_data, m_capacity + 1);
                }
                m_size = 0;
                m_capacity = 0;
                m_using_sso = true;
                m_sso_data[0] = '\0';
            }

            void push_back(char c) {
                if (m_size + 1 <= SSO_THRESHOLD) {
                    // Can fit in SSO buffer
                    if (m_using_sso) {
                        m_sso_data[m_size] = c;
                        m_sso_data[m_size + 1] = '\0';
                        ++m_size;
                    } else {
                        // Need to switch to SSO
                        char* old_data = m_data;
                        size_t old_capacity = m_capacity;
                        std::memcpy(m_sso_data, old_data, m_size);
                        m_sso_data[m_size] = c;
                        m_sso_data[m_size + 1] = '\0';
                        ++m_size;
                        m_using_sso = true;
                        m_allocator.deallocate(old_data, old_capacity + 1);
                    }
                } else {
                    // Need heap allocation
                    if (m_using_sso) {
                        // Switch from SSO to heap
                        size_t new_capacity = (m_size + 1) * 2;
                        char* new_data = m_allocator.allocate(new_capacity + 1);
                        std::memcpy(new_data, m_sso_data, m_size);
                        new_data[m_size] = c;
                        new_data[m_size + 1] = '\0';
                        m_data = new_data;
                        m_capacity = new_capacity;
                        ++m_size;
                        m_using_sso = false;
                    } else {
                        // Already on heap, check if we need to grow
                        if (m_size + 1 > m_capacity) {
                            size_t new_capacity = (m_size + 1) * 2;
                            char* new_data = m_allocator.allocate(new_capacity + 1);
                            std::memcpy(new_data, m_data, m_size);
                            new_data[m_size] = c;
                            new_data[m_size + 1] = '\0';
                            m_allocator.deallocate(m_data, m_capacity + 1);
                            m_data = new_data;
                            m_capacity = new_capacity;
                            ++m_size;
                        } else {
                            m_data[m_size] = c;
                            m_data[m_size + 1] = '\0';
                            ++m_size;
                        }
                    }
                }
            }

            void replace(size_t pos, size_t len, const char *str)
            {
                assert(pos + len <= m_size);
                KSafeString temp = substr(0, pos);
                temp += str;
                temp += substr(pos + len, m_size - pos - len);
                *this = temp;
            }

            KSafeString &operator+=(const char *str)
            {
                size_t str_len = std::strlen(str);
                size_t new_len = m_size + str_len;
                
                if (new_len <= SSO_THRESHOLD)
                {
                    // Can fit in SSO buffer
                    if (m_using_sso)
                    {
                        std::strcat(m_sso_data, str);
                    }
                    else
                    {
                        // Copy from heap to SSO
                        char temp[SSO_THRESHOLD + 1];
                        std::strcpy(temp, m_data);
                        std::strcat(temp, str);
                        
                        // Clean up heap allocation
                        m_allocator.deallocate(m_data, m_capacity + 1);
                        
                        // Copy to SSO buffer
                        std::strcpy(m_sso_data, temp);
                        m_using_sso = true;
                    }
                    m_size = new_len;
                }
                else
                {
                    // Need heap allocation
                    char *new_data = m_allocator.allocate(new_len + 1);
                    std::strcpy(new_data, c_str());
                    std::strcat(new_data, str);
                    
                    // Clean up old allocation
                    if (!m_using_sso) 
                    {
                        m_allocator.deallocate(m_data, m_capacity + 1);
                    }
                    
                    m_data      = new_data;
                    m_capacity  = new_len;
                    m_size      = new_len;
                    m_using_sso = false;
                }
                return *this;
            }

            KSafeString &operator+=(char c)
            {
                char str[2] = {c, '\0'};
                return *this += str;
            }

            KSafeString &operator+=(const KSafeString& other)
            {
                return *this += other.c_str();
            }

            const char &operator[](size_t i) const
            {
                assert(i < m_size);
                return m_using_sso ? m_sso_data[i] : m_data[i];
            }

            char &operator[](size_t i)
            {
                assert(i < m_size);
                return m_using_sso ? m_sso_data[i] : m_data[i];
            }

            const char* begin() const { return c_str(); }
            const char* end() const { return c_str() + m_size; }

            char* begin() { return const_cast<char*>(c_str()); }
            char* end() { return const_cast<char*>(c_str() + m_size); }


        private:
            union
            {
                    char  m_sso_data[SSO_THRESHOLD + 1]; // +1 for null terminator
                    char *m_data;
            };
            size_t m_size      = 0;
            size_t m_capacity  = 0;
            bool   m_using_sso = true;
            Alloc  m_allocator;
    };

    // Global operator+ for const char* + KSafeString
    template<typename Alloc>
    KSafeString<Alloc> operator+(const char* lhs, const KSafeString<Alloc>& rhs) {
        KSafeString<Alloc> result(lhs);
        result += rhs;
        return result;
    }
    
}