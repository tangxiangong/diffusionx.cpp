module;

#include <cmath>
#include <complex>
#include <fftw3.h>
#include <vector>

export module diffusionx.simulation.continuous.fbm;

import diffusionx.error;
import diffusionx.random.normal;
import diffusionx.simulation.basic.abstract;
import diffusionx.simulation.basic.utils;

using std::complex;
using std::vector;

/**
 * @brief Fractional Brownian Motion implementation
 *
 * This class implements fractional Brownian motion (fBm), a generalization of
 * Brownian motion with long-range dependence controlled by the Hurst parameter.
 *
 * Properties:
 * - B_H(0) = 0
 * - E[B_H(t)] = 0
 * - E[B_H(t)²] = t^(2H)
 * - Covariance: E[B_H(s)B_H(t)] = 0.5 * (s^(2H) + t^(2H) - |t-s|^(2H))
 *
 * where H ∈ (0, 1) is the Hurst parameter:
 * - H = 0.5: standard Brownian motion
 * - H > 0.5: persistent (positively correlated increments)
 * - H < 0.5: anti-persistent (negatively correlated increments)
 */
export class FBM : public ContinuousProcess {
  double m_hurst = 0.5;          ///< Hurst parameter H ∈ (0, 1)
  double m_start_position = 0.0; ///< Initial position

public:
  /**
   * @brief Default constructor creating standard Brownian motion (H = 0.5)
   */
  FBM() = default;

  /**
   * @brief Constructs fBm with specified parameters
   * @param hurst Hurst parameter H (must be in (0, 1))
   * @param start_position Initial position
   * @throws std::invalid_argument if hurst is not in (0, 1)
   */
  FBM(double hurst, double start_position = 0.0)
      : m_hurst(hurst), m_start_position(start_position) {
    if (m_hurst <= 0.0 || m_hurst >= 1.0) {
      throw std::invalid_argument("Hurst parameter must be in (0, 1)");
    }
  }

  /**
   * @brief Gets the Hurst parameter
   * @return The Hurst parameter H
   */
  [[nodiscard]] auto get_hurst() const -> double { return m_hurst; }

  /**
   * @brief Gets the initial position
   * @return The initial position
   */
  [[nodiscard]] auto get_start_position() const -> double {
    return m_start_position;
  }

  /**
   * @brief Simulates a trajectory of the fBm using circulant embedding
   * @param duration The total simulation time
   * @param time_step The time step for discretization
   * @return Result containing time and position vectors, or an Error
   *
   * Uses the circulant embedding method with FFT for exact simulation
   * of the Gaussian process with the fBm covariance structure.
   */
  Result<vec_pair> simulate(double duration, double time_step = 0.01) override {
    if (duration <= 0) {
      return Err(Error::InvalidArgument("Duration must be positive"));
    }
    if (time_step <= 0) {
      return Err(Error::InvalidArgument("Time step must be positive"));
    }

    size_t n = static_cast<size_t>(std::ceil(duration / time_step));

    // Generate fBm using circulant embedding
    auto fbm_result = generate_fbm_circulant_embedding(n, m_hurst);
    if (!fbm_result.has_value()) {
      return Err(fbm_result.error());
    }

    auto fbm_increments = fbm_result.value();

    // Create time and position vectors
    vector<double> times(n + 1);
    vector<double> positions(n + 1);

    times[0] = 0.0;
    positions[0] = m_start_position;

    for (size_t i = 1; i <= n; ++i) {
      times[i] = i * time_step;
      positions[i] = m_start_position +
                     fbm_increments[i - 1] * std::pow(time_step, m_hurst);
    }

    return Ok(std::make_pair(std::move(times), std::move(positions)));
  }

private:
  /**
   * @brief Generates fBm using circulant embedding method
   * @param n Number of time steps
   * @param hurst Hurst parameter
   * @return Result containing fBm values, or an Error
   */
  Result<vector<double>> generate_fbm_circulant_embedding(size_t n,
                                                          double hurst) {
    // Compute covariance function for fBm
    auto covariance = [hurst](double k) -> double {
      if (k == 0)
        return 1.0;
      return 0.5 * (std::pow(std::abs(k + 1), 2 * hurst) +
                    std::pow(std::abs(k - 1), 2 * hurst) -
                    2 * std::pow(std::abs(k), 2 * hurst));
    };

    // Create circulant matrix (first row)
    size_t m = 2 * n; // Size for circulant embedding
    vector<double> c(m);

    for (size_t i = 0; i < n; ++i) {
      c[i] = covariance(static_cast<double>(i));
    }
    for (size_t i = n; i < m; ++i) {
      c[i] = covariance(static_cast<double>(m - i));
    }

    // Compute eigenvalues using FFT
    vector<complex<double>> c_complex(m);
    for (size_t i = 0; i < m; ++i) {
      c_complex[i] = complex<double>(c[i], 0.0);
    }

    fftw_complex *in = reinterpret_cast<fftw_complex *>(c_complex.data());
    fftw_complex *out = fftw_alloc_complex(m);
    fftw_plan plan = fftw_plan_dft_1d(static_cast<int>(m), in, out,
                                      FFTW_FORWARD, FFTW_ESTIMATE);

    fftw_execute(plan);

    // Check if all eigenvalues are non-negative
    vector<double> eigenvalues(m);
    for (size_t i = 0; i < m; ++i) {
      eigenvalues[i] = out[i][0];    // Real part
      if (eigenvalues[i] < -1e-10) { // Allow small numerical errors
        fftw_destroy_plan(plan);
        fftw_free(out);
        return Err(Error::InvalidArgument(
            "Circulant matrix is not positive semidefinite"));
      }
      eigenvalues[i] = std::max(0.0, eigenvalues[i]); // Ensure non-negative
    }

    // Generate random Gaussian vector
    auto gaussian_result = randn(m, 0.0, 1.0);
    if (!gaussian_result.has_value()) {
      fftw_destroy_plan(plan);
      fftw_free(out);
      return Err(gaussian_result.error());
    }
    auto gaussian = gaussian_result.value();

    // Apply square root of eigenvalues
    vector<complex<double>> z(m);
    for (size_t i = 0; i < m; ++i) {
      z[i] = complex<double>(std::sqrt(eigenvalues[i]) * gaussian[i], 0.0);
    }

    // Inverse FFT
    fftw_complex *z_in = reinterpret_cast<fftw_complex *>(z.data());
    fftw_complex *z_out = fftw_alloc_complex(m);
    fftw_plan inv_plan = fftw_plan_dft_1d(static_cast<int>(m), z_in, z_out,
                                          FFTW_BACKWARD, FFTW_ESTIMATE);

    fftw_execute(inv_plan);

    // Extract the first n values and scale
    vector<double> result(n);
    double scale = 1.0 / std::sqrt(static_cast<double>(m));
    for (size_t i = 0; i < n; ++i) {
      result[i] = z_out[i][0] * scale; // Real part
    }

    // Cleanup
    fftw_destroy_plan(plan);
    fftw_destroy_plan(inv_plan);
    fftw_free(out);
    fftw_free(z_out);

    return Ok(std::move(result));
  }

public:
  /**
   * @brief Computes the theoretical covariance between two time points
   * @param s First time point
   * @param t Second time point
   * @return The theoretical covariance Cov[B_H(s), B_H(t)]
   */
  [[nodiscard]] auto theoretical_covariance(double s, double t) const
      -> double {
    return 0.5 * (std::pow(s, 2 * m_hurst) + std::pow(t, 2 * m_hurst) -
                  std::pow(std::abs(t - s), 2 * m_hurst));
  }

  /**
   * @brief Computes the theoretical variance at time t
   * @param t Time point
   * @return The theoretical variance Var[B_H(t)]
   */
  [[nodiscard]] auto theoretical_variance(double t) const -> double {
    return std::pow(t, 2 * m_hurst);
  }
};