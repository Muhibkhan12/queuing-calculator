/*
 * ==========================================================================
 * queue_models.h
 * --------------------------------------------------------------------------
 * Header file declaring the calculation functions for the Queueing Model
 * Calculator (Qt GUI version).
 *
 * Unlike a console program, these functions do NOT read input with cin or
 * print with cout. Instead, each function:
 *   1. Receives its parameters as plain function arguments (already read
 *      from the Qt QLineEdit boxes by mainwindow.cpp)
 *   2. Returns a QueueResults struct containing the computed performance
 *      measures
 *   3. Reports errors (invalid input / unstable system) through the
 *      "errorMessage" out-parameter instead of printing to the console
 *
 * This keeps the calculation logic completely independent of Qt, so it
 * could be reused in a console app, a test suite, or a different GUI
 * without any changes.
 *
 * Each model corresponds to standard queueing theory notation:
 *   M/M/1     - Single server, infinite capacity, infinite population
 *   M/M/s     - Multiple servers, infinite capacity
 *   M/M/inf   - Infinite servers (self-service system)
 *   M/M/1/K   - Single server, finite capacity K
 *   M/M/s/K   - Multiple servers, finite capacity K
 *   M/G/1     - Single server, general service time distribution
 *               (solved using the Pollaczek-Khinchine formula)
 * ==========================================================================
 */

#ifndef QUEUE_MODELS_H
#define QUEUE_MODELS_H

#include <string>

/*
 * Struct to hold the common performance measures computed for
 * (almost) every queueing model. Not every field is meaningful for
 * every model; unused fields are simply left at their default values
 * (-1.0) and the GUI layer skips displaying them when so.
 *
 * Fields:
 *   rho             - Utilization factor (server occupancy)
 *   P0              - Probability that the system is empty
 *   L               - Expected number of customers in the system
 *   Lq              - Expected number of customers waiting in the queue
 *   W               - Expected time a customer spends in the system
 *   Wq              - Expected time a customer spends waiting in the queue
 *   Pblock          - Blocking probability (finite capacity models only)
 *   throughput      - Effective arrival rate accepted into the system
 *   isValid         - false if the inputs were invalid or the system is
 *                      unstable; when false, the other fields should be
 *                      ignored and errorMessage explains why
 *   errorMessage    - Human-readable explanation when isValid is false
 *                      (e.g. "System is unstable: lambda >= mu")
 */
struct QueueResults {
    double rho          = -1.0;
    double P0           = -1.0;
    double L            = -1.0;
    double Lq           = -1.0;
    double W            = -1.0;
    double Wq           = -1.0;
    double Pblock       = -1.0;
    double throughput   = -1.0;
    bool   isValid      = true;
    std::string errorMessage;
};

// ---------------------------------------------------------------------
// Model calculation functions - each one validates its own inputs,
// checks system stability, computes performance measures, and returns
// them in a QueueResults struct. No console I/O is performed here.
// ---------------------------------------------------------------------

// M/M/1 : Single-server queue with infinite capacity and infinite population
//   lambda - arrival rate
//   mu     - service rate
QueueResults calculateMM1(double lambda, double mu);

// M/M/s : Multi-server queue with infinite capacity
//   lambda - arrival rate
//   mu     - service rate (per server)
//   s      - number of servers
QueueResults calculateMMS(double lambda, double mu, int s);

// M/M/infinity : Infinite-server queue (no waiting ever occurs)
//   lambda - arrival rate
//   mu     - service rate (per server)
QueueResults calculateMMInfinite(double lambda, double mu);

// M/M/1/K : Single-server queue with finite system capacity K
//   lambda - arrival rate
//   mu     - service rate
//   K      - maximum number of customers allowed in the system
QueueResults calculateMM1K(double lambda, double mu, int K);

// M/M/s/K : Multi-server queue with finite system capacity K
//   lambda - arrival rate
//   mu     - service rate (per server)
//   s      - number of servers
//   K      - maximum number of customers allowed in the system (K >= s)
QueueResults calculateMMSK(double lambda, double mu, int s, int K);

// M/G/1 : Single-server queue with general service time distribution,
//         solved via the Pollaczek-Khinchine (P-K) formula
//   lambda        - arrival rate
//   mu            - service rate (mean service time = 1/mu)
//   serviceVariance - variance of the service time distribution (sigma^2)
QueueResults calculateMG1(double lambda, double mu, double serviceVariance);

#endif // QUEUE_MODELS_H