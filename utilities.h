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
// Time unit conversion
// ---------------------------------------------------------------------
// Many queueing problems give the arrival rate and service rate in
// DIFFERENT time units (e.g. "5 customers per hour" but "1 customer
// every 4 minutes"). Before the formulas in queue_models.cpp can be
// used, both rates must be expressed in the SAME time unit.
//
// This calculator standardizes everything internally to "per minute"
// (an arbitrary but consistent choice), converting from whichever unit
// and input style (rate vs. mean time) the user selected.

// The three time units the GUI lets the user choose from for each of
// lambda (arrival) and mu (service).
enum class TimeUnit {
    Hours,
    Minutes,
    Seconds
};

// Converts a duration expressed in "unit" into an equivalent duration
// in minutes. E.g. convertToMinutes(1.0, TimeUnit::Hours) -> 60.0
double convertToMinutes(double value, TimeUnit unit);

// Converts a user-entered value for lambda or mu into a standardized
// "events per minute" rate, based on:
//   isMean - true if "inputValue" is a MEAN TIME (e.g. average minutes
//            between arrivals / average minutes to serve one customer),
//            false if "inputValue" is already a RATE (e.g. customers
//            per hour)
//   unit   - the time unit "inputValue" was expressed in
//
// Conversion logic:
//   Rate input:  ratePerMinute = rate_in_unit / (unit's minutes-per-unit)
//                e.g. 5 customers/hour  -> 5 / 60 = 0.0833 customers/min
//   Mean input:  first convert the mean time to minutes, then invert:
//                ratePerMinute = 1 / meanTimeInMinutes
//                e.g. mean = 4 minutes/customer -> 1/4 = 0.25 customers/min
double convertToRatePerMinute(double inputValue, bool isMean, TimeUnit unit);

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