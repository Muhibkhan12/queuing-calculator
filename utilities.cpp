/*
 * ==========================================================================
 * utilities.cpp
 * --------------------------------------------------------------------------
 * Implements the helper functions declared in utilities.h:
 *   - Safe parsing of user-entered text into numeric values
 *   - Consistent fixed-decimal formatting of results for display
 * ==========================================================================
 */

#include "utilities.h"
#include <sstream>
#include <iomanip>
#include <cctype>

// ==========================================================================
// tryParseDouble
// --------------------------------------------------------------------------
// Uses std::istringstream to attempt a full numeric conversion of "text".
// We explicitly check that:
//   (a) the stream extraction succeeded (no malformed number), and
//   (b) the ENTIRE string was consumed (rejects things like "12abc"
//       which istringstream would otherwise happily parse as 12)
// ==========================================================================
bool tryParseDouble(const std::string &text, double &outValue)
{
    if (text.empty()) {
        return false;
    }

    std::istringstream stream(text);
    double parsedValue = 0.0;

    stream >> parsedValue;

    // Fail if extraction failed, or if there are leftover characters
    // after the number (ignoring trailing whitespace).
    if (stream.fail()) {
        return false;
    }

    std::string remainder;
    stream >> remainder;
    if (!remainder.empty()) {
        return false; // extra non-numeric characters were present
    }

    outValue = parsedValue;
    return true;
}

// ==========================================================================
// tryParsePositiveInt
// --------------------------------------------------------------------------
// First checks that every character is a digit (rejecting decimals,
// signs, and letters), then parses the integer and confirms it is > 0.
// ==========================================================================
bool tryParsePositiveInt(const std::string &text, int &outValue)
{
    if (text.empty()) {
        return false;
    }

    for (char character : text) {
        if (!std::isdigit(static_cast<unsigned char>(character))) {
            return false; // contains a non-digit character (e.g. '.', '-', 'a')
        }
    }

    std::istringstream stream(text);
    int parsedValue = 0;
    stream >> parsedValue;

    if (stream.fail()) {
        return false;
    }

    if (parsedValue <= 0) {
        return false; // number of servers / capacity must be positive
    }

    outValue = parsedValue;
    return true;
}

// ==========================================================================
// isPositiveNumber
// --------------------------------------------------------------------------
// Simple check: value must be strictly greater than zero.
// ==========================================================================
bool isPositiveNumber(double value)
{
    return value > 0.0;
}

// ==========================================================================
// formatFixed
// --------------------------------------------------------------------------
// Uses std::ostringstream with std::fixed and std::setprecision to render
// "value" with exactly "decimalPlaces" digits after the decimal point.
// ==========================================================================
std::string formatFixed(double value, int decimalPlaces)
{
    std::ostringstream stream;
    stream << std::fixed << std::setprecision(decimalPlaces) << value;
    return stream.str();
}

// ==========================================================================
// formatMeasure
// --------------------------------------------------------------------------
// QueueResults uses -1.0 as a sentinel value to mean "this performance
// measure does not apply to the selected model" (e.g. Pblock for M/M/1,
// which has no finite capacity). We detect that sentinel here and print
// "N/A" instead of a misleading "-1.0000".
// ==========================================================================
std::string formatMeasure(double value, int decimalPlaces)
{
    // Use a small epsilon comparison since -1.0 is exactly representable,
    // but this guards against any future floating-point drift.
    const double sentinelEpsilon = 1e-9;
    if (value < 0.0 && (value + 1.0) > -sentinelEpsilon && (value + 1.0) < sentinelEpsilon) {
        return "N/A";
    }

    return formatFixed(value, decimalPlaces);
}