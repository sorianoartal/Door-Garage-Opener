#pragma once
#include <Arduino.h>
#include "Config/CC1101_Config/CC1101.h"

/// @brief Namespace for AVR algorithms and utilities
/// @note This namespace contains various utility functions and algorithms that can be used in AVR-based applications
/// such as repeating operations, iterating over arrays, and finding elements in collections.
/// @note It provides a set of generic algorithms that can be applied to arrays, containers, and iterators.
/// @namespace avr_algorithms	
namespace avr_algorithms {

/**
 * @brief Repeats an operation a specified number of times.
 * * This function takes an integer 'n' and a callable 'operation', and executes the operation 'n' times.
 * * @note This is useful for scenarios where you need to perform an action multiple times, such as sending a signal or performing a task repeatedly.
 * * @example	
 *   repeat(5, []() {
 * 	  Serial.println("Hello, World!");
 *   });
 * 
 * @tparam Func - Type of the callable operation to be executed repeatedly
 * @note The 'Func' type can be a lambda, function pointer, or any callable
 * @param n - The number of times to repeat the operation
 * @note The 'n' parameter must be a non-negative integer. If 'n'
 * @param operation - The operation to be executed repeatedly
 * @note The 'operation' parameter is a callable that takes no arguments and returns void.
 */
template<typename Func>
void repeat(int n, Func&& operation) {
    while (n-- > 0) {
        operation();
    }    
}

/**
 * @brief Repeats an operation with an exit condition.
 * * This function takes an integer 'n' and a callable 'operation', and executes the operation up to 'n' times or until the operation returns false.
 * * @note This is useful for scenarios where you want to repeat an action until a certain condition is met, such as waiting for a signal or checking a status.
 * * @example
 *   repeat_withExitCondition(5, []() {
 * 		Serial.println("Checking status...");
 * 		return checkStatus(); // Returns true to continue, false to exit
 *   });
 * 
 * @tparam Func - Type of the callable operation to be executed repeatedly
 * * @note The 'Func' type can be a lambda, function pointer, or any callable that returns a boolean value
 * @param n - The maximum number of times to repeat the operation
 * * @note The 'n' parameter must be a non-negative integer. If 'n' is less than or equal to zero, the operation will not be executed.
 * @param operation - The operation to be executed repeatedly
 * * @note The 'operation' parameter is a callable that takes no arguments and returns a boolean value.
 * * @note The operation should return true to continue repeating, or false to exit the loop
 */
template<typename Func>
void repeat_withExitCondition(int n, Func&& operation) {
    while (n-- > 0) {
        if (!operation()) break;
    }    
}

/**
 * @brief Converts a CC1101 command to its string representation.
 * * This function takes a CC1101 command enum value and returns its string representation.
 * * @note This is useful for debugging or logging purposes, where you want to display the command name instead of its numeric value.
 * * @example
 *   Serial.println(toString(CC1101::Strobes::Command::SRES)); // Outputs "SRES"
 * 
 * @param cmd - The CC1101 command enum value to convert
 * @return const char* - The string representation of the command
 */
inline const char* toString(CC1101::Strobes::Command cmd) {
    using Command = CC1101::Strobes::Command;
    switch (cmd) {
        case Command::SRES:     return "SRES";
        case Command::SFSTXON:  return "SFSTXON";
        case Command::SXOFF:    return "SXOFF";
        case Command::SCAL:     return "SCAL";
        case Command::SRX:      return "SRX";
        case Command::STX:      return "STX";
        case Command::SIDLE:    return "SIDLE";
        case Command::SWOR:     return "SWOR";
        case Command::SPWD:     return "SPWD";
        case Command::SFRX:     return "SFRX";
        case Command::SFTX:     return "SFTX";
        case Command::SWORRST:  return "SWORRST";
        case Command::SNOP:     return "SNOP";
        default:                return "UNKNOWN";
    }
}

/**
 * @brief Applies a function to each element of an array or buffer.
 * * This function iterates over the elements of an array or buffer and applies a specified function to each element.
 * * @note This is useful for scenarios where you want to perform an operation on each element, such as processing data or modifying values.
 * * @example
 *   int arr[] = {1, 2, 3, 4, 5};
 *   for_each(arr, [](int& value, size_t index) {
 *       value *= 2; // Double each value
 *   });
 * 
 * @tparam T - Type of the elements in the array or buffer
 * @tparam N - Size of the array (if applicable)
 * @tparam Func - Type of the callable function to apply to each element
 * @note The 'Func' type can be a lambda, function pointer, or any callable that takes an element and its index as arguments
 * @param array - The array or buffer to iterate over
 * @param func - The function to apply to each element
 */
template<typename T, size_t N, typename Func>
void for_each(T(&array)[N], Func&& func) {
    for (size_t i = 0; i < N; ++i) {
        func(array[i], i);
    }
}

/**
 * @brief Applies a function to each element of an array or buffer without the index.
 * * This function iterates over the elements of an array or buffer and applies a specified function to each element without providing the index.
 * * @note This is useful for scenarios where you want to process each element independently, such as transforming data or applying a filter.
 * * @example
 *   int arr[] = {1, 2, 3, 4, 5};
 *   for_each_element(arr, [](int& value) {
 *       value += 10; // Add 10 to each value
 *   });
 * 
 * @tparam T - Type of the elements in the array or buffer
 * @tparam N - Size of the array (if applicable)
 * @tparam Func - Type of the callable function to apply to each element
 * @note The 'Func' type can be a lambda, function pointer, or any callable that takes an element as an argument
 * @param array - The array or buffer to iterate over
 * @param func - The function to apply to each element
 */
template<typename T, size_t N, typename Func>
void for_each_element(T(&array)[N], Func&& func) {
    for (size_t i = 0; i < N; ++i) {
        func(array[i]);
    }
}

/**
 * @brief Applies a function to each element of a buffer with an index.
 * * This function iterates over the elements of a buffer and applies a specified function to each element, providing the index as an additional argument.
 * * @note This is useful for scenarios where you need to know the position of each element while processing it, such as logging or debugging.
 * * @example
 *   uint8_t buffer[] = {10, 20, 30, 40};
 *   for_each(buffer, sizeof(buffer), [](uint8_t& value, uint8_t index) {
 *       Serial.print("Index: "); Serial.print(index); Serial.print(", Value: "); Serial.println(value);
 *   });
 * 
 * @tparam T - Type of the elements in the buffer
 * @tparam Func - Type of the callable function to apply to each element
 * @note The 'Func' type can be a lambda, function pointer, or any callable that takes an element and its index as arguments
 * @param buffer - Pointer to the buffer to iterate over
 * @param length - Length of the buffer
 * @param func - The function to apply to each element
 */
template<typename T, typename Func>
void for_each(T* buffer, size_t length, Func&& func) {
    for (size_t i = 0; i < length; ++i) {
        func(buffer[i], static_cast<uint8_t>(i));
    }
}

/**
 * @brief Applies a function to each element of a buffer without the index.
 * * This function iterates over the elements of a buffer and applies a specified function to each element without providing the index.
 * * @note This is useful for scenarios where you want to process each element independently, such as transforming data or applying a filter.
 * * @example
 *   uint8_t buffer[] = {10, 20, 30, 40};
 *   for_each_element(buffer, sizeof(buffer), [](uint8_t& value) {
 *       value += 5; // Add 5 to each value
 *   });
 * 
 * @tparam T - Type of the elements in the buffer
 * @tparam Func - Type of the callable function to apply to each element
 * @note The 'Func' type can be a lambda, function pointer, or any callable that takes an element as an argument
 * @param buffer - Pointer to the buffer to iterate over
 * @param length - Length of the buffer
 * @param func - The function to apply to each element
 */
template<typename T, typename Func>
inline void for_each(const T* buffer, size_t length, Func func) {
    for (size_t i = 0; i < length; ++i) {
        func(buffer[i]);
    }
}


/**
 * @brief Applies a function to each element of a buffer until the function returns false.
 * * This function iterates over the elements of a buffer and applies a specified function to each element until the function returns false.
 * * @note This is useful for scenarios where you want to stop processing elements as soon as a certain condition is met, such as finding a specific value or checking a condition.
 * * @example
 *   uint8_t buffer[] = {1, 2, 3, 4, 5};
 *   bool allLessThanTen = for_each_until(buffer, sizeof(buffer), [](uint8_t& value) {
 *       return value < 10; // Continue until a value is not less than 10
 *   });
 * 
 * @tparam T - Type of the elements in the buffer
 * @tparam Func - Type of the callable function to apply to each element
 * @note The 'Func' type can be a lambda, function pointer, or any callable that takes an element as an argument and returns a boolean value
 * @param buffer - Pointer to the buffer to iterate over
 * @param length - Length of the buffer
 * @param func - The function to apply to each element
 * @return bool - Returns true if all elements satisfied the condition, false otherwise
 */
template<typename T, typename Func>
inline void for_each_element(const T* buffer, size_t length, Func func) {
    for (size_t i = 0; i < length; ++i) {
        func(buffer[i]);
    }
}


/**
 * @brief Applies a function to each element of a buffer until the function returns false.
 * * This function iterates over the elements of a buffer and applies a specified function to each element until the function returns false.
 * * @note This is useful for scenarios where you want to stop processing elements as soon as a certain condition is met, such as finding a specific value or checking a condition.
 * * @example
 *   uint8_t buffer[] = {1, 2, 3, 4, 5};
 *   bool allLessThanTen = for_each_until(buffer, sizeof(buffer), [](uint8_t& value) {
 *       return value < 10; // Continue until a value is not less than 10
 *   });
 * 
 * @tparam T - Type of the elements in the buffer
 * @tparam Func - Type of the callable function to apply to each element
 * @note The 'Func' type can be a lambda, function pointer, or any callable that takes an element as an argument and returns a boolean value
 * @param buffer - Pointer to the buffer to iterate over
 * @param length - Length of the buffer
 * @param func - The function to apply to each element
 * @return bool - Returns true if all elements satisfied the condition, false otherwise
 */
template<typename T, typename Func>
inline bool for_each_until(const T* buffer, size_t length, Func func) {
    for (size_t i = 0; i < length; ++i) {
        if (!func(buffer[i])) return false;
    }
    return true;
}

/**
 * @brief Applies a function to each element of a range defined by iterators.
 * * This function iterates over the elements of a range defined by two iterators and applies a specified function to each element.
 * * @note This is useful for scenarios where you want to process elements in a range, such as filtering or transforming data.
 * * @example
 *   std::vector<int> vec = {1, 2, 3, 4, 5};
 *   for_each(vec.begin(), vec.end(), [](int& value) {
 *       value *= 2; // Double each value
 *   });
 * 
 * @tparam Iterator - Type of the iterator used to traverse the range
 * @tparam Func - Type of the callable function to apply to each element
 * @note The 'Func' type can be a lambda, function pointer, or any callable that takes an element as an argument
 * @param begin - Iterator pointing to the beginning of the range
 * @param end - Iterator pointing to the end of the range
 * @param func - The function to apply to each element
 */
template<typename Iterator, typename Func>
void for_each(Iterator begin, Iterator end, Func&& func) {
    for (auto it = begin; it != end; ++it) {
        func(*it);
    }
}

/**
 * @brief Applies a function to each element of a container.
 * * This function iterates over the elements of a container (like std::vector, std::list, etc.) and applies a specified function to each element.
 * * @note This is useful for scenarios where you want to process elements in a container, such as filtering or transforming data.
 * * @example
 *   std::vector<int> vec = {1, 2, 3, 4, 5};
 *   for_each(vec, [](int& value) {
 *       value += 10; // Add 10 to each value
 *   });
 * 
 * @tparam Container - Type of the container to iterate over
 * @tparam Func - Type of the callable function to apply to each element
 * @note The 'Func' type can be a lambda, function pointer, or any callable that takes an element as an argument
 * @param container - The container to iterate over
 * @param func - The function to apply to each element
 */
template<typename Container, typename Func>
void for_each(const Container& container, Func&& func) {
    for (auto it = container.begin(); it != container.end(); ++it) {
        func(*it);
    }
}

/**
 * @brief Applies a function to each element of an array.
 * * This function iterates over the elements of an array and applies a specified function to each element.
 * * @note This is useful for scenarios where you want to process elements in an array, such as filtering or transforming data.
 * * @example
 *  int arr[] = {1, 2, 3, 4, 5};
 * 	auto value = find_if(arr, [](int& value) {
 * 	 	return value > 3; // Find first element greater than 3
 * 	});
 * 
 * @tparam T - Type of the elements in the array
 * @tparam N - Size of the array
 * @tparam Func - Type of the callable function to apply to each element
 * @note The 'Func' type can be a lambda, function pointer, or any callable that takes an element as an argument
 * @param array - The array to iterate over
 * @param func - The function to apply to each element
 */
template<typename Iterator, typename Predicate>
Iterator find_if_impl(Iterator begin, Iterator end, Predicate&& predicate) {
    for (auto it = begin; it != end; ++it) {
        if (predicate(*it)) {
            return it;
        }
    }
    return end;
}

/**
 * @brief Finds the first element in a range that satisfies a given predicate.
 * * This function iterates over the elements of a range defined by two iterators and returns an iterator to the first element that satisfies the predicate.
 * * @note This is useful for scenarios where you want to find an element based on a condition, such as searching for a specific value or property.
 * * @example
 *   std::vector<int> vec = {1, 2, 3, 4, 5};
 *   auto it = find_if(vec.begin(), vec.end(), [](int value) {
 *       return value > 3; // Find first element greater than 3
 *   });
 * 
 * @tparam Iterator - Type of the iterator used to traverse the range
 * @tparam Predicate - Type of the callable predicate to check each element
 * @note The 'Predicate' type can be a lambda, function pointer, or any callable that takes an element as an argument and returns a boolean value
 * @param beginIt - Iterator pointing to the beginning of the range
 * @param endIt - Iterator pointing to the end of the range
 * @param predicate - The predicate function to check each element
 * @return Iterator - An iterator to the first element that satisfies the predicate, or 'endIt' if no such element is found
 */
template<typename Iterator, typename Predicate>
Iterator find_if(Iterator beginIt, Iterator endIt, Predicate&& predicate) {
    return find_if_impl(beginIt, endIt, predicate);
}

/**
 * @brief Finds the first element in an array that satisfies a given predicate.
 * * This function iterates over the elements of an array and returns a pointer to the first element that satisfies the predicate.
 * * @note This is useful for scenarios where you want to find an element based on a condition, such as searching for a specific value or property.
 * * @example
 *   int arr[] = {1, 2, 3, 4, 5};
 *   auto value = find_if(arr, [](int& value) {
 *       return value > 3; // Find first element greater than 3
 *   });
 * 
 * @tparam T - Type of the elements in the array
 * @tparam N - Size of the array
 * @tparam Predicate - Type of the callable predicate to check each element
 * @note The 'Predicate' type can be a lambda, function pointer, or any callable that takes an element as an argument and returns a boolean value
 * @param array - The array to iterate over
 * @param predicate - The predicate function to check each element
 * @return T* - A pointer to the first element that satisfies the predicate, or 'nullptr' if no such element is found
 */
template<typename T, size_t N, typename Predicate>
T* find_if(T(&array)[N], Predicate&& predicate) {
    return find_if_impl(array, array + N, predicate);
}


/**
 * @brief Counts the number of occurrences of a specific value in a range defined by iterators.
 * * This function iterates over the elements of a range defined by two iterators and counts how many times a specific value appears.
 * * @note This is useful for scenarios where you want to count occurrences of a value, such as finding duplicates or checking frequency.
 * * @example
 *   std::vector<int> vec = {1, 2, 3, 2, 4};
 *   size_t count = count(vec.begin(), vec.end(), 2); // Count occurrences of 2
 * 
 * @tparam Iterator - Type of the iterator used to traverse the range
 * @tparam T - Type of the value to count
 * @note The 'T' type can be any type that supports equality comparison
 * @param begin - Iterator pointing to the beginning of the range
 * @param end - Iterator pointing to the end of the range
 * @param value - The value to count occurrences of
 * @return size_t - The number of occurrences of the specified value in the range
 */
template<typename Iterator, typename T>
size_t count_impl(Iterator begin, Iterator end, const T& value) {
    size_t count = 0;
    for (auto it = begin; it != end; ++it) {
        if (*it == value) {
            ++count;
        }
    }
    return count;
}

/**
 * @brief Counts the number of occurrences of a specific value in a range defined by iterators.
 * * This function iterates over the elements of a range defined by two iterators and counts how many times a specific value appears.
 * * @note This is useful for scenarios where you want to count occurrences of a value, such as finding duplicates or checking frequency.
 * * @example
 *   std::vector<int> vec = {1, 2, 3, 2, 4};
 *   size_t count = count(vec.begin(), vec.end(), 2); // Count occurrences of 2
 * 
 * @tparam Iterator - Type of the iterator used to traverse the range
 * @tparam T - Type of the value to count
 * @note The 'T' type can be any type that supports equality comparison
 * @param begin - Iterator pointing to the beginning of the range
 * @param end - Iterator pointing to the end of the range
 * @param value - The value to count occurrences of
 * @return size_t - The number of occurrences of the specified value in the range
 */
template<typename Iterator, typename T>
size_t count(Iterator begin, Iterator end, const T& value) {
    return count_impl(begin, end, value);
}


/**
 * @brief Counts the number of occurrences of a specific value in an array.
 * * This function iterates over the elements of an array and counts how many times a specific value appears.
 * * @note This is useful for scenarios where you want to count occurrences of a value, such as finding duplicates or checking frequency.
 * * @example
 *   int arr[] = {1, 2, 3, 2, 4};
 *   size_t count = count(arr, 2); // Count occurrences of 2
 * 
 * @tparam T - Type of the elements in the array
 * @tparam N - Size of the array
 * @note The 'T' type can be any type that supports equality comparison
 * @param array - The array to iterate over
 * @param value - The value to count occurrences of
 * @return size_t - The number of occurrences of the specified value in the array
 */
template<typename T, size_t N>
size_t count(const T(&array)[N], const T& value) {
    return count_impl(array, array + N, value);
}

/**
 * @brief Counts the number of elements in a range that satisfy a given predicate.
 * * This function iterates over the elements of a range defined by two iterators and counts how many elements satisfy a specified predicate.
 * * @note This is useful for scenarios where you want to count elements based on a condition, such as filtering or checking properties.
 * * @example
 *   std::vector<int> vec = {1, 2, 3, 4, 5};
 *   size_t count = count_if(vec.begin(), vec.end(), [](int value) {
 *       return value > 2; // Count elements greater than 2
 *   });
 * 
 * @tparam Iterator - Type of the iterator used to traverse the range
 * @tparam Predicate - Type of the callable predicate to check each element
 * @note The 'Predicate' type can be a lambda, function pointer, or any callable that takes an element as an argument and returns a boolean value
 * @param begin - Iterator pointing to the beginning of the range
 * @param end - Iterator pointing to the end of the range
 * @param predicate - The predicate function to check each element
 * @return size_t - The number of elements that satisfy the predicate in the range
 */
template<typename Iterator, typename Predicate>
size_t count_if_impl(Iterator begin, Iterator end, Predicate&& predicate) {
    size_t count = 0;
    for (auto it = begin; it != end; ++it) {
        if (predicate(*it)) {
            ++count;
        }
    }
    return count;
}

/**
 * @brief Counts the number of elements in a range that satisfy a given predicate.
 * * This function iterates over the elements of a range defined by two iterators and counts how many elements satisfy a specified predicate.
 * * @note This is useful for scenarios where you want to count elements based on a condition, such as filtering or checking properties.
 * * @example
 *   std::vector<int> vec = {1, 2, 3, 4, 5};
 *   size_t count = count_if(vec.begin(), vec.end(), [](int value) {
 *       return value > 2; // Count elements greater than 2
 *   });
 * 
 * @tparam Iterator - Type of the iterator used to traverse the range
 * @tparam Predicate - Type of the callable predicate to check each element
 * @note The 'Predicate' type can be a lambda, function pointer, or any callable that takes an element as an argument and returns a boolean value
 * @param begin - Iterator pointing to the beginning of the range
 * @param end - Iterator pointing to the end of the range
 * @param predicate - The predicate function to check each element
 * @return size_t - The number of elements that satisfy the predicate in the range
 */
template<typename Iterator, typename Predicate>
size_t count_if(Iterator begin, Iterator end, Predicate&& predicate) {
    return count_if_impl(begin, end, predicate);
}


/**
 * @brief Counts the number of elements in an array that satisfy a given predicate.
 * * This function iterates over the elements of an array and counts how many elements satisfy a specified predicate.
 * * @note This is useful for scenarios where you want to count elements based on a condition, such as filtering or checking properties.
 * * @example
 *   int arr[] = {1, 2, 3, 4, 5};
 *   size_t count = count_if(arr, [](int value) {
 *       return value > 2; // Count elements greater than 2
 *   });
 * 
 * @tparam T - Type of the elements in the array
 * @tparam N - Size of the array
 * @tparam Predicate - Type of the callable predicate to check each element
 * @note The 'Predicate' type can be a lambda, function pointer, or any callable that takes an element as an argument and returns a boolean value
 * @param array - The array to iterate over
 * @param predicate - The predicate function to check each element
 * @return size_t - The number of elements that satisfy the predicate in the array
 */
template<typename T, size_t N, typename Predicate>
size_t count_if(const T(&array)[N], Predicate&& predicate) {
    return count_if_impl(array, array + N, predicate);
}

/**
 * @brief Copies elements from one range to another with a maximum count.
 * * This function copies elements from a source range defined by two iterators to a destination iterator, up to a specified maximum count.
 * * @note This is useful for scenarios where you want to copy a limited number of elements, such as transferring data between buffers or containers.
 * * @example
 *   std::vector<int> src = {1, 2, 3, 4, 5};
 *   std::vector<int> dest(3);
 *   unsigned copied = copy(src.begin(), src.end(), dest.begin(), 3); // Copy up to 3 elements
 * 
 * @tparam Iterator - Type of the iterator used to traverse the source range
 * @tparam OutputIterator - Type of the iterator used for the destination
 * @note The 'OutputIterator' type can be any output iterator that supports assignment
 * @param begin - Iterator pointing to the beginning of the source range
 * @param end - Iterator pointing to the end of the source range
 * @param dest_first - Iterator pointing to the beginning of the destination range
 * @param max_count - Maximum number of elements to copy
 * @return OutputIterator - An iterator pointing to the end of the copied elements in the destination range
 */
template<typename Iterator, typename OutputIterator>
OutputIterator copy_impl(Iterator begin, Iterator end, OutputIterator dest_first, unsigned max_count) {
    unsigned copied_count = 0;
    for (auto it = begin; it != end && copied_count < max_count; ++it, ++dest_first, ++copied_count) {
        *dest_first = *it;
    }
    return dest_first;
}


/**
 * @brief Copies elements from one range to another with a maximum count.
 * * This function copies elements from a source range defined by two iterators to a destination iterator, up to a specified maximum count.
 * * @note This is useful for scenarios where you want to copy a limited number of elements, such as transferring data between buffers or containers.
 * * @example
 *   std::vector<int> src = {1, 2, 3, 4, 5};
 *   std::vector<int> dest(3);
 *   unsigned copied = copy(src.begin(), src.end(), dest.begin(), 3); // Copy up to 3 elements
 * 
 * @tparam Iterator - Type of the iterator used to traverse the source range
 * @tparam OutputIterator - Type of the iterator used for the destination
 * @note The 'OutputIterator' type can be any output iterator that supports assignment
 * @param begin - Iterator pointing to the beginning of the source range
 * @param end - Iterator pointing to the end of the source range
 * @param dest_first - Iterator pointing to the beginning of the destination range
 * @param dest_size - Maximum number of elements to copy
 * @return unsigned - The number of elements actually copied
 */
template<typename Iterator, typename OutputIterator>
unsigned copy(Iterator begin, Iterator end, OutputIterator dest_first, unsigned dest_size) {
    if (end - begin > dest_size) {
        end = begin + dest_size;
    }
    unsigned copied_count = (end - begin < dest_size) ? (end - begin) : dest_size;
    copy_impl(begin, end, dest_first, copied_count);
    return copied_count;
}

/**
 * @brief Copies elements from a container to a destination iterator with a maximum count.
 * * This function copies elements from a container to a destination iterator, up to a specified maximum count.
 * * @note This is useful for scenarios where you want to copy a limited number of elements from a container, such as transferring data between buffers or containers.
 * * @example
 *   std::vector<int> src = {1, 2, 3, 4, 5};
 *   std::vector<int> dest(3);
 *   unsigned copied = copy(src, dest.begin(), 3); // Copy up to 3 elements
 * 
 * @tparam Container - Type of the container to copy from
 * @tparam OutputIterator - Type of the iterator used for the destination
 * @note The 'OutputIterator' type can be any output iterator that supports assignment
 * @param container - The container to copy from
 * @param dest_first - Iterator pointing to the beginning of the destination range
 * @param dest_size - Maximum number of elements to copy
 * @return unsigned - The number of elements actually copied
 */
template<typename Container, typename OutputIterator>
unsigned copy(const Container& container, OutputIterator dest_first, unsigned dest_size) {
    if (container.empty() || dest_size == 0) {
        return 0;
    }
    unsigned elements_to_copy = (container.size() < dest_size) ? container.size() : dest_size;
    copy_impl(container.begin(), container.end(), dest_first, elements_to_copy);
    return elements_to_copy;
}


/**
 * @brief Copies elements from an array to a destination iterator with a maximum count.
 * * This function copies elements from an array to a destination iterator, up to a specified maximum count.
 * * @note This is useful for scenarios where you want to copy a limited number of elements from an array, such as transferring data between buffers or containers.
 * * @example
 *   int arr[] = {1, 2, 3, 4, 5};
 *   std::vector<int> dest(3);
 *   unsigned copied = copy(arr, dest.begin(), 3); // Copy up to 3 elements
 * 
 * @tparam T - Type of the elements in the array
 * @tparam N - Size of the array
 * @tparam OutputIterator - Type of the iterator used for the destination
 * @note The 'OutputIterator' type can be any output iterator that supports assignment
 * @param array - The array to copy from
 * @param dest_first - Iterator pointing to the beginning of the destination range
 * @param dest_size - Maximum number of elements to copy
 * @return unsigned - The number of elements actually copied
 */
template<typename T, size_t N, typename OutputIterator>
unsigned copy(const T(&array)[N], OutputIterator dest_first, unsigned dest_size) {
    unsigned elements_to_copy = (N < dest_size) ? N : dest_size;
    copy_impl(array, array + elements_to_copy, dest_first, elements_to_copy);
    return elements_to_copy;
}

/**
 * @brief Copies elements from a source array to a destination array with a maximum count.
 * * This function copies elements from a source array to a destination array, up to the size of the destination array.
 * * @note This is useful for scenarios where you want to copy elements between arrays, such as transferring data or initializing arrays.
 * * @example
 *   int src[] = {1, 2, 3, 4, 5};
 *   int dest[3];
 *   copy(src, dest, 3); // Copy up to 3 elements
 * 
 * @tparam T - Type of the elements in the arrays
 * @tparam N - Size of the source array
 * @param src - The source array to copy from
 * @param dest - The destination array to copy to
 * @param dest_size - Maximum number of elements to copy
 * @return T* - Pointer to the end of the copied elements in the destination array
 */
template<typename T, size_t N>
T* copy(const T(&src)[N], T(&dest)[N], unsigned dest_size) {
    unsigned elements_to_copy = (N < dest_size) ? N : dest_size;
    copy_impl(src, src + elements_to_copy, dest, elements_to_copy);
    return dest + elements_to_copy;
}

/**
 * @brief Removes elements from a range based on a predicate.
 * * This function iterates over the elements of a range defined by two iterators and removes elements that satisfy a specified predicate.
 * * @note This is useful for scenarios where you want to filter out elements based on a condition, such as removing unwanted values or properties.
 * * @example
 *   std::vector<int> vec = {1, 2, 3, 4, 5};
 *   auto new_end = remove_if(vec.begin(), vec.end(), [](int value) {
 *       return value % 2 == 0; // Remove even numbers
 *   });
 *   vec.erase(new_end, vec.end()); // Erase the removed elements
 * 
 * @tparam Iterator - Type of the iterator used to traverse the range
 * @tparam Predicate - Type of the callable predicate to check each element
 * @note The 'Predicate' type can be a lambda, function pointer, or any callable that takes an element as an argument and returns a boolean value
 * @param begin - Iterator pointing to the beginning of the range
 * @param end - Iterator pointing to the end of the range
 * @param predicate - The predicate function to check each element
 * @return Iterator - An iterator pointing to the new end of the range after removal
 */
template<typename Iterator, typename Predicate>
Iterator remove_if_impl(Iterator begin, Iterator end, Predicate&& predicate) {
    auto write_it = begin;
    for (auto it = begin; it != end; ++it) {
        if (!predicate(*it)) {
            *write_it = *it;
            ++write_it;
        }
    }
    return write_it;
}


/**
 * @brief Removes elements from a range based on a predicate.
 * * This function iterates over the elements of a range defined by two iterators and removes elements that satisfy a specified predicate.
 * * @note This is useful for scenarios where you want to filter out elements based on a condition, such as removing unwanted values or properties.
 * * @example
 *   std::vector<int> vec = {1, 2, 3, 4, 5};
 *   auto new_end = remove_if(vec.begin(), vec.end(), [](int value) {
 *       return value % 2 == 0; // Remove even numbers
 *   });
 *   vec.erase(new_end, vec.end()); // Erase the removed elements
 * 
 * @tparam Iterator - Type of the iterator used to traverse the range
 * @tparam Predicate - Type of the callable predicate to check each element
 * @note The 'Predicate' type can be a lambda, function pointer, or any callable that takes an element as an argument and returns a boolean value
 * @param begin - Iterator pointing to the beginning of the range
 * @param end - Iterator pointing to the end of the range
 * @param predicate - The predicate function to check each element
 * @return Iterator - An iterator pointing to the new end of the range after removal
 */
template<typename Iterator, typename Predicate>
Iterator remove_if(Iterator begin, Iterator end, Predicate&& predicate) {
    return remove_if_impl(begin, end, predicate);
}

template<typename T, size_t N, typename Predicate>
T* remove_if(T(&array)[N], Predicate&& predicate) {
    return remove_if_impl(array, array + N, predicate);
}

} // namespace avr_algorithms