/*
================================================================================
  VEK (Vantor Engine Kernel) - Used by Vantor Studios
--------------------------------------------------------------------------------
  Author  : Lukas Rennhofer (lukas.renn@aon.at)
  License : GNU General Public License v3.0

  “Order the chaos, frame the void — and call it a world.”
================================================================================
*/

// This is a simplified std::vector-like container with basic dynamic array functionality
// Provides efficient memory management with allocator support

#pragma once

#include <cassert>
#include <cstdint>
#include <initializer_list>
#include <memory>
#include <new>
#include <utility>

namespace VEK::Core
{
    // Custom KVector container template class
    // T: element type, A: allocator type (defaults to std::allocator<T>)
    template <typename T, typename A = std::allocator<T>> class KVector
    {
        public:
            using value_type = T;

            // Default constructor - creates empty KVector or with specified initial size
            inline KVector(size_t initial_size = 0) { resize(initial_size); }

            // Copy constructor - creates deep copy of another KVector
            inline KVector(const KVector<T> &source_KVector) { copy_from(source_KVector); }

            // Move constructor - transfers ownership from another KVector
            inline KVector(KVector<T> &&source_KVector) noexcept { move_from(std::move(source_KVector)); }

            // Initializer list constructor - creates KVector from brace-enclosed list
            inline KVector(std::initializer_list<T> init_list)
            {
                reserve(init_list.size());
                for (auto &element : init_list)
                {
                    push_back(element);
                }
            }

            // Destructor - cleans up all elements and deallocates memory
            inline ~KVector()
            {
                clear();
                if (m_data_ptr != nullptr)
                {
                    m_allocator.deallocate(m_data_ptr, m_capacity);
                }
            }

            // Copy assignment - replaces contents with deep copy of another KVector
            inline KVector<T> &operator=(const KVector<T> &source_KVector)
            {
                copy_from(source_KVector);
                return *this;
            }

            // Move assignment - replaces contents by transferring ownership
            inline KVector<T> &operator=(KVector<T> &&source_KVector)
            {
                move_from(std::move(source_KVector));
                return *this;
            }

            // Array subscript operator - provides direct access to element at index (no bounds checking)
            constexpr T &operator[](size_t element_index) noexcept
            {
                assert(m_current_size > 0 && element_index < m_current_size);
                return m_data_ptr[element_index];
            }

            // Const array subscript operator - provides read-only access to element at index
            constexpr const T &operator[](size_t element_index) const noexcept
            {
                assert(m_current_size > 0 && element_index < m_current_size);
                return m_data_ptr[element_index];
            }

            // Reserve memory for at least the specified number of elements (doesn't change size)
            inline void reserve(size_t requested_capacity)
            {
                if (requested_capacity > m_capacity)
                {
                    size_t old_capacity = m_capacity;
                    T     *old_data_ptr = allocate_and_move(requested_capacity);
                    if (old_data_ptr != nullptr)
                    {
                        m_allocator.deallocate(old_data_ptr, old_capacity);
                    }
                }
            }

            // Resize KVector to contain exactly the specified number of elements
            inline void resize(size_t new_size)
            {
                if (new_size > m_current_size)
                {
                    reserve(new_size);

                    // Expand the KVector - default-construct new elements
                    for (size_t i = m_current_size; i < new_size; ++i)
                    {
                        new (m_data_ptr + i) T();
                    }
                    m_current_size = new_size;
                }
                else if (new_size < m_current_size)
                {
                    // Shrink the KVector - destroy excess elements
                    while (m_current_size > new_size)
                    {
                        pop_back();
                    }
                }
            }

            // Reduce capacity to match current size (frees unused memory)
            inline void shrink_to_fit()
            {
                if (m_capacity > m_current_size)
                {
                    size_t old_capacity = m_capacity;
                    T     *old_data_ptr = allocate_and_move(m_current_size);
                    if (old_data_ptr != nullptr)
                    {
                        m_allocator.deallocate(old_data_ptr, old_capacity);
                    }
                }
            }

            // Construct element in-place at the end of the KVector using provided arguments
            template <typename... ConstructorArgs> inline T &emplace_back(ConstructorArgs &&...constructor_args)
            {
                size_t       old_capacity      = m_capacity;
                T           *old_data_ptr      = nullptr;
                const size_t required_capacity = m_current_size + 1;

                // Check if we need to grow the capacity
                if (required_capacity > m_capacity)
                {
                    // Double the capacity to amortize future allocations
                    old_data_ptr = allocate_and_move(required_capacity * 2);
                }

                // Construct new element at the end
                T *new_element_ptr = new (m_data_ptr + m_current_size) T(std::forward<ConstructorArgs>(constructor_args)...);
                m_current_size++;

                // Clean up old allocation if we reallocated
                if (old_data_ptr != nullptr)
                {
                    m_allocator.deallocate(old_data_ptr, old_capacity);
                }
                return *new_element_ptr;
            }

            // Add element to the end of the KVector (copy version)
            inline void push_back(const T &element) { emplace_back(element); }

            // Add element to the end of the KVector (move version)
            inline void push_back(T &&element) { emplace_back(std::move(element)); }

            // Remove all elements from the KVector (calls destructors but doesn't deallocate memory)
            constexpr void clear() noexcept
            {
                for (size_t i = 0; i < m_current_size; ++i)
                {
                    m_data_ptr[i].~T();
                }
                m_current_size = 0;
            }

            // Remove the last element from the KVector
            constexpr void pop_back() noexcept
            {
                assert(m_current_size > 0);
                m_data_ptr[--m_current_size].~T();
            }

            // Remove element at specified position (shifts remaining elements left)
            constexpr void erase(T *position) noexcept
            {
                assert(position >= begin());
                assert(position < end());
                std::move(position + 1, end(), position); // Shift elements left to fill gap
                pop_back();                               // Destroy the now-duplicate last element
            }

            // Get reference to the last element
            constexpr T &back() noexcept
            {
                assert(m_current_size > 0);
                return m_data_ptr[m_current_size - 1];
            }

            // Get const reference to the last element
            constexpr const T &back() const noexcept
            {
                assert(m_current_size > 0);
                return m_data_ptr[m_current_size - 1];
            }

            // Get reference to the first element
            constexpr T &front() noexcept
            {
                assert(m_current_size > 0);
                return m_data_ptr[0];
            }

            // Get const reference to the first element
            constexpr const T &front() const noexcept
            {
                assert(m_current_size > 0);
                return m_data_ptr[0];
            }

            // Get direct pointer to underlying array
            constexpr T *data() noexcept { return m_data_ptr; }

            // Get const pointer to underlying array
            constexpr const T *data() const noexcept { return m_data_ptr; }

            // Bounds-checked element access (same as operator[] but with explicit assertion)
            constexpr T &at(size_t element_index) noexcept
            {
                assert(m_current_size > 0 && element_index < m_current_size);
                return m_data_ptr[element_index];
            }

            // Const bounds-checked element access
            constexpr const T &at(size_t element_index) const noexcept
            {
                assert(m_current_size > 0 && element_index < m_current_size);
                return m_data_ptr[element_index];
            }

            // Get the number of elements currently in the KVector
            constexpr size_t size() const noexcept { return m_current_size; }

            // Check if the KVector contains no elements
            constexpr bool empty() const noexcept { return m_current_size == 0; }

            // Get iterator to first element
            constexpr T *begin() noexcept { return m_data_ptr; }

            // Get const iterator to first element
            constexpr const T *begin() const noexcept { return m_data_ptr; }

            // Get iterator to one past the last element
            constexpr T *end() noexcept { return m_data_ptr + m_current_size; }

            // Get const iterator to one past the last element
            constexpr const T *end() const noexcept { return m_data_ptr + m_current_size; }

        private:
            // Allocate new memory and move existing elements to it
            // Returns pointer to old data if reallocation occurred, nullptr otherwise
            inline T *allocate_and_move(size_t new_capacity)
            {
                if (new_capacity > m_current_size)
                {
                    m_capacity = new_capacity;

                    // Allocate new memory block
                    T *new_allocation = m_allocator.allocate(m_capacity);

                    // Move existing elements to new location
                    for (size_t i = 0; i < m_current_size; ++i)
                    {
                        new (new_allocation + i) T(std::move(m_data_ptr[i]));
                    }

                    // Swap data pointers and return old allocation for cleanup
                    std::swap(m_data_ptr, new_allocation);
                    return new_allocation;
                }
                return nullptr;
            }

            // Copy all elements from another KVector (used by copy constructor and assignment)
            inline void copy_from(const KVector<T> &source_KVector)
            {
                resize(source_KVector.size());
                for (size_t i = 0; i < m_current_size; ++i)
                {
                    m_data_ptr[i] = source_KVector.m_data_ptr[i];
                }
            }

            // Transfer ownership from another KVector (used by move constructor and assignment)
            inline void move_from(KVector<T> &&source_KVector)
            {
                clear();
                if (m_data_ptr != nullptr)
                {
                    m_allocator.deallocate(m_data_ptr, m_capacity);
                }

                // Take ownership of source KVector's data
                m_capacity     = source_KVector.m_capacity;
                m_current_size = source_KVector.m_current_size;
                m_data_ptr     = source_KVector.m_data_ptr;

                // Leave source KVector in valid empty state
                source_KVector.m_data_ptr     = nullptr;
                source_KVector.m_current_size = 0;
                source_KVector.m_capacity     = 0;
            }

            // Members
            T     *m_data_ptr     = nullptr;
            size_t m_current_size = 0;
            size_t m_capacity     = 0;
            A      m_allocator;
    };
}