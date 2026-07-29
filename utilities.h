/*
 * ==========================================================================
 * utilities.h
 * --------------------------------------------------------------------------
 * Header file declaring general-purpose helper functions used across the
 * Queueing Model Calculator:
 *   - Parsing and validating numeric text entered by the user
 *   - Formatting numeric results as strings with a fixed number of
 *     decimal places for consistent display
 *
 * These helpers use plain std::string (not QString) so this file has no
 * dependency on Qt and can be reused/tested independently of the GUI.
 * mainwindow.cpp converts QString <-> std::string at the boundary using
 * QString::toStdString() / QString::fromStdString().
 * ==========================================================================
 */

#ifndef UTILITIES_H
#define UTILITIES_H

#include <string>

// ---------------------------------------------------------------------
// Input validation / parsing helpers
// ---------------------------------------------------------------------

// Attempts to parse "text" as a double.
// Returns true and stores the parsed value in "outValue" on success.
// Returns false if "text" is empty, malformed, or contains extra
// non-numeric characters (e.g. "12abc").
bool tryParseDouble(const std::string &text, double &outValue);

// Attempts to parse "text" as a positive integer (> 0).
// Returns true and stores the parsed value in "outValue" on success.
// Returns false if "text" is empty, malformed, not an integer, or <= 0.
bool tryParsePositiveInt(const std::string &text, int &outValue);

// Returns true if "value" is strictly greater than zero.
// Used to validate arrival rate (lambda), service rate (mu), and
// service time variance before passing them into the queueing formulas.
bool isPositiveNumber(double value);

// ---------------------------------------------------------------------
// Output formatting helpers
// ---------------------------------------------------------------------

// Formats "value" as a fixed-point decimal string with exactly
// "decimalPlaces" digits after the decimal point (default 4), e.g.
// formatFixed(0.5) -> "0.5000"
std::string formatFixed(double value, int decimalPlaces = 4);

// Formats a performance-measure value for display. If "value" is the
// QueueResults sentinel (-1.0, meaning "not applicable" for this model),
// returns "N/A" instead of a misleading formatted number.
std::string formatMeasure(double value, int decimalPlaces = 4);

#endif // UTILITIES_H