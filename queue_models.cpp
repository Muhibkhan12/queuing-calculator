/*
 * ==========================================================================
 * queue_models.cpp
 * --------------------------------------------------------------------------
 * Implements the calculation functions declared in queue_models.h.
 *
 * Each function performs three steps:
 *   1. Validate inputs and check system stability
 *   2. Apply the relevant queueing theory formulas
 *   3. Package the results into a QueueResults struct and return it
 *
 * No console or GUI I/O happens here - this file is pure calculation
 * logic, so it can be safely called from mainwindow.cpp (Qt GUI) without
 * any dependency on Qt headers.
 * ==========================================================================
 */

#include "queue_models.h"
#include <cmath>

// Helper: computes n! (factorial) as a double to avoid integer overflow
// for reasonably large n (used in M/M/s and M/M/s/K Erlang formulas).
static double factorial(int n)
{
    double result = 1.0;
    for (int i = 2; i <= n; ++i) {
        result *= i;
    }
    return result;
}

// ==========================================================================
// M/M/1 : Single server, infinite capacity, infinite population
// ==========================================================================
QueueResults calculateMM1(double lambda, double mu)
{
    QueueResults r;

    // ---- Input validation ----
    if (lambda <= 0.0 || mu <= 0.0) {
        r.isValid = false;
        r.errorMessage = "Arrival rate and service rate must both be positive.";
        return r;
    }

    // ---- Stability check ----
    // For M/M/1, the system reaches steady state only if lambda < mu
    // (utilization rho = lambda/mu must be strictly less than 1).
    if (lambda >= mu) {
        r.isValid = false;
        r.errorMessage = "System is unstable: arrival rate (lambda) must be "
                          "less than service rate (mu).";
        return r;
    }

    // rho = lambda / mu  --> server utilization (fraction of time server is busy)
    r.rho = lambda / mu;

    // P0 = 1 - rho  --> probability the system is empty
    r.P0 = 1.0 - r.rho;

    // L = rho / (1 - rho)  --> expected number of customers in the system
    r.L = r.rho / (1.0 - r.rho);

    // Lq = rho^2 / (1 - rho)  --> expected number of customers in the queue
    r.Lq = (r.rho * r.rho) / (1.0 - r.rho);

    // W = 1 / (mu - lambda)  --> expected time in system (Little's Law: L = lambda * W)
    r.W = 1.0 / (mu - lambda);

    // Wq = lambda / (mu * (mu - lambda))  --> expected time waiting in queue
    r.Wq = lambda / (mu * (mu - lambda));

    // Throughput equals lambda since every arriving customer is served
    // (infinite capacity, no blocking).
    r.throughput = lambda;

    return r;
}

// ==========================================================================
// M/M/s : Multiple servers, infinite capacity
// ==========================================================================
QueueResults calculateMMS(double lambda, double mu, int s)
{
    QueueResults r;

    // ---- Input validation ----
    if (lambda <= 0.0 || mu <= 0.0 || s <= 0) {
        r.isValid = false;
        r.errorMessage = "Arrival rate, service rate, and number of servers "
                          "must all be positive.";
        return r;
    }

    // ---- Stability check ----
    // For M/M/s, the total service capacity is s * mu. The system is
    // stable only if lambda < s * mu (rho = lambda / (s*mu) < 1).
    if (lambda >= s * mu) {
        r.isValid = false;
        r.errorMessage = "System is unstable: arrival rate (lambda) must be "
                          "less than total service capacity (s * mu).";
        return r;
    }

    // rho = lambda / (s * mu)  --> per-server utilization
    r.rho = lambda / (s * mu);

    // a = lambda / mu  --> "offered load" in Erlangs, used inside the
    // Erlang-C formula below
    double a = lambda / mu;

    // ---- Compute P0 using the Erlang-C formula ----
    // P0 = [ sum_{n=0}^{s-1} (a^n / n!) + (a^s / s!) * (1 / (1 - rho)) ]^-1
    double sumTerms = 0.0;
    for (int n = 0; n < s; ++n) {
        sumTerms += std::pow(a, n) / factorial(n);
    }
    double lastTerm = std::pow(a, s) / factorial(s) * (1.0 / (1.0 - r.rho));
    r.P0 = 1.0 / (sumTerms + lastTerm);

    // ---- Erlang-C probability of waiting (Pwait) ----
    // Pwait = (a^s / (s! * (1 - rho))) * P0
    double Pwait = (std::pow(a, s) / factorial(s)) * (1.0 / (1.0 - r.rho)) * r.P0;

    // Lq = Pwait * rho / (1 - rho)  --> expected number waiting in queue
    r.Lq = Pwait * r.rho / (1.0 - r.rho);

    // L = Lq + a  --> expected number in system (queue + being served)
    r.L = r.Lq + a;

    // Wq = Lq / lambda  --> expected wait time in queue (Little's Law)
    r.Wq = r.Lq / lambda;

    // W = Wq + 1/mu  --> expected time in system (waiting + service time)
    r.W = r.Wq + 1.0 / mu;

    // Throughput equals lambda (infinite capacity, no blocking)
    r.throughput = lambda;

    return r;
}

// ==========================================================================
// M/M/infinity : Infinite servers (self-service system, no waiting)
// ==========================================================================
QueueResults calculateMMInfinite(double lambda, double mu)
{
    QueueResults r;

    // ---- Input validation ----
    if (lambda <= 0.0 || mu <= 0.0) {
        r.isValid = false;
        r.errorMessage = "Arrival rate and service rate must both be positive.";
        return r;
    }

    // With infinite servers, every arriving customer is served immediately,
    // so this system is ALWAYS stable regardless of lambda and mu.

    // rho is not a meaningful "server busy fraction" here since there are
    // infinite servers; by convention we report it as lambda/mu (offered load)
    r.rho = lambda / mu;

    // P0 = e^(-lambda/mu)  --> probability system is empty (Poisson with mean lambda/mu)
    r.P0 = std::exp(-lambda / mu);

    // L = lambda / mu  --> expected number in system (Poisson distributed)
    r.L = lambda / mu;

    // Lq = 0 always, since there are always enough servers, no one ever waits
    r.Lq = 0.0;

    // W = 1 / mu  --> time in system equals pure service time (no waiting)
    r.W = 1.0 / mu;

    // Wq = 0 always, no waiting occurs
    r.Wq = 0.0;

    // Throughput equals lambda (every customer is served immediately)
    r.throughput = lambda;

    return r;
}

// ==========================================================================
// M/M/1/K : Single server, finite system capacity K
// ==========================================================================
QueueResults calculateMM1K(double lambda, double mu, int K)
{
    QueueResults r;

    // ---- Input validation ----
    if (lambda <= 0.0 || mu <= 0.0 || K <= 0) {
        r.isValid = false;
        r.errorMessage = "Arrival rate, service rate, and capacity K must "
                          "all be positive.";
        return r;
    }

    // Note: M/M/1/K is ALWAYS stable, even if lambda >= mu, because the
    // finite capacity K guarantees the queue can never grow unbounded.
    // We do not reject lambda >= mu here; blocking probability absorbs
    // the excess demand instead.

    // rho = lambda / mu  --> traffic intensity (may be >= 1 for finite queues)
    r.rho = lambda / mu;

    // ---- Compute P0 ----
    // If rho != 1:  P0 = (1 - rho) / (1 - rho^(K+1))
    // If rho == 1:  P0 = 1 / (K+1)   (special case, uniform distribution)
    if (std::abs(r.rho - 1.0) < 1e-9) {
        r.P0 = 1.0 / (K + 1);
    } else {
        r.P0 = (1.0 - r.rho) / (1.0 - std::pow(r.rho, K + 1));
    }

    // ---- Blocking probability P_K ----
    // P_K = rho^K * P0  (if rho != 1), or P_K = P0 (if rho == 1)
    // This is the probability the system is full (K customers present),
    // so any arriving customer is blocked (turned away).
    if (std::abs(r.rho - 1.0) < 1e-9) {
        r.Pblock = r.P0;
    } else {
        r.Pblock = std::pow(r.rho, K) * r.P0;
    }

    // ---- Expected number in system L ----
    // If rho != 1:
    //   L = rho / (1-rho) - (K+1)*rho^(K+1) / (1 - rho^(K+1))
    // If rho == 1:
    //   L = K / 2   (uniform distribution over 0..K)
    if (std::abs(r.rho - 1.0) < 1e-9) {
        r.L = K / 2.0;
    } else {
        double rhoK1 = std::pow(r.rho, K + 1);
        r.L = r.rho / (1.0 - r.rho) - (K + 1) * rhoK1 / (1.0 - rhoK1);
    }

    // Effective arrival rate (throughput): only non-blocked customers enter
    // lambda_eff = lambda * (1 - P_K)
    r.throughput = lambda * (1.0 - r.Pblock);

    // Lq = L - (1 - P0)  --> number in queue = number in system minus
    // the expected number actually being served (server busy fraction = 1-P0)
    r.Lq = r.L - (1.0 - r.P0);

    // W = L / lambda_eff  --> expected time in system (Little's Law using
    // the effective/accepted arrival rate, since blocked customers never enter)
    r.W = r.L / r.throughput;

    // Wq = Lq / lambda_eff  --> expected waiting time in queue
    r.Wq = r.Lq / r.throughput;

    return r;
}

// ==========================================================================
// M/M/s/K : Multiple servers, finite system capacity K (K >= s)
// ==========================================================================
QueueResults calculateMMSK(double lambda, double mu, int s, int K)
{
    QueueResults r;

    // ---- Input validation ----
    if (lambda <= 0.0 || mu <= 0.0 || s <= 0 || K <= 0) {
        r.isValid = false;
        r.errorMessage = "Arrival rate, service rate, servers, and capacity "
                          "must all be positive.";
        return r;
    }
    if (K < s) {
        r.isValid = false;
        r.errorMessage = "System capacity K must be greater than or equal "
                          "to the number of servers (s).";
        return r;
    }

    // Note: M/M/s/K is ALWAYS stable regardless of lambda vs s*mu, because
    // the finite capacity K guarantees bounded queue length.

    // a = lambda / mu  --> offered load in Erlangs
    double a = lambda / mu;

    // rho = lambda / (s * mu)  --> per-server utilization (informational;
    // does not need to be < 1 for stability since K is finite)
    r.rho = lambda / (s * mu);

    // ---- Compute P0 ----
    // P0 = [ sum_{n=0}^{s-1} (a^n / n!) + sum_{n=s}^{K} (a^n / (s! * s^(n-s))) ]^-1
    double sumBelowS = 0.0;
    for (int n = 0; n < s; ++n) {
        sumBelowS += std::pow(a, n) / factorial(n);
    }
    double sumFromSToK = 0.0;
    for (int n = s; n <= K; ++n) {
        sumFromSToK += std::pow(a, n) / (factorial(s) * std::pow((double)s, n - s));
    }
    r.P0 = 1.0 / (sumBelowS + sumFromSToK);

    // ---- Helper lambda: probability of exactly n customers in system, Pn ----
    // For n < s:  Pn = (a^n / n!) * P0
    // For s<=n<=K: Pn = (a^n / (s! * s^(n-s))) * P0
    auto Pn = [&](int n) -> double {
        if (n < s) {
            return (std::pow(a, n) / factorial(n)) * r.P0;
        } else {
            return (std::pow(a, n) / (factorial(s) * std::pow((double)s, n - s))) * r.P0;
        }
    };

    // ---- Blocking probability P_K ----
    // The probability an arriving customer is blocked equals P_K
    // (system is at full capacity K) by the PASTA property.
    r.Pblock = Pn(K);

    // ---- Expected number waiting in queue Lq ----
    // Lq = sum_{n=s}^{K} (n - s) * Pn
    r.Lq = 0.0;
    for (int n = s; n <= K; ++n) {
        r.Lq += (n - s) * Pn(n);
    }

    // Effective arrival rate (throughput): only accepted customers enter
    // lambda_eff = lambda * (1 - P_K)
    r.throughput = lambda * (1.0 - r.Pblock);

    // ---- Expected number in system L ----
    // L = Lq + (lambda_eff / mu)  --> queue length plus average number
    // of customers actually being served
    r.L = r.Lq + (r.throughput / mu);

    // Wq = Lq / lambda_eff  --> expected waiting time in queue (Little's Law)
    r.Wq = r.Lq / r.throughput;

    // W = Wq + 1/mu  --> expected total time in system
    r.W = r.Wq + 1.0 / mu;

    return r;
}

// ==========================================================================
// M/G/1 : Single server, general service time distribution
//         Solved using the Pollaczek-Khinchine (P-K) formula
// ==========================================================================
QueueResults calculateMG1(double lambda, double mu, double serviceVariance)
{
    QueueResults r;

    // ---- Input validation ----
    if (lambda <= 0.0 || mu <= 0.0 || serviceVariance < 0.0) {
        r.isValid = false;
        r.errorMessage = "Arrival rate and service rate must be positive, "
                          "and service time variance cannot be negative.";
        return r;
    }

    // ---- Stability check ----
    // rho = lambda * E[S] = lambda / mu must be < 1
    if (lambda >= mu) {
        r.isValid = false;
        r.errorMessage = "System is unstable: arrival rate (lambda) must be "
                          "less than service rate (mu).";
        return r;
    }

    // rho = lambda / mu  --> server utilization
    r.rho = lambda / mu;

    // P0 = 1 - rho  --> probability the system is empty
    // (this holds for M/G/1 just as it does for M/M/1)
    r.P0 = 1.0 - r.rho;

    // ---- Pollaczek-Khinchine (P-K) formula for Lq ----
    // Lq = (lambda^2 * E[S^2] + rho^2) / (2 * (1 - rho))
    // where E[S^2] = Var(S) + (E[S])^2 = serviceVariance + (1/mu)^2
    // This is commonly written as:
    //   Lq = (lambda^2 * sigma^2 + rho^2) / (2 * (1 - rho))
    double meanServiceSquared = serviceVariance + std::pow(1.0 / mu, 2);
    r.Lq = (lambda * lambda * meanServiceSquared) / (2.0 * (1.0 - r.rho));

    // Wq = Lq / lambda  --> expected waiting time in queue (Little's Law)
    r.Wq = r.Lq / lambda;

    // W = Wq + 1/mu  --> expected total time in system
    r.W = r.Wq + 1.0 / mu;

    // L = lambda * W  --> expected number in system (Little's Law)
    r.L = lambda * r.W;

    // Throughput equals lambda (infinite capacity, no blocking)
    r.throughput = lambda;

    return r;
}