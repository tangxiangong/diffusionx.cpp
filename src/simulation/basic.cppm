module;

#include <memory>
#include <utility>
#include <vector>
#include <concepts>

export module diffusionx.simulation.basic;

import diffusionx.error;

using std::vector;

export using vec_pair = std::pair<vector<double>, vector<double>>;

export using double_pair = std::pair<double, double>;

export struct ContinuousProcess {
    virtual ~ContinuousProcess() = default;
    virtual Result<vec_pair> simulate(double duration,
                                      double time_step = 0.01) = 0;
    virtual Result<double> mean(double duration, size_t particles = 10000,
                                double time_step = 0.01);
    virtual Result<double> msd(double duration, size_t particles = 10000,
                               double time_step = 0.01);
    virtual Result<double> raw_moment(double duration, int order,
                                      size_t particles = 10000,
                                      double time_step = 0.01);
    virtual Result<double> central_moment(double duration, int order,
                                          size_t particles = 10000,
                                          double time_step = 0.01);
    virtual Result<Option<double>> fpt(double_pair domain,
                                       double max_duration = 1000,
                                       double time_step = 0.01);
    virtual Result<double> occupation_time(double_pair domain, double duration,
                                           double time_step = 0.01);
    virtual Result<double> tamsd(double duration, double delta,
                                 size_t quad_order = 10,
                                 double time_step = 0.01);
    virtual Result<double> eatamsd(double duration, double delta,
                                   size_t particles = 10000,
                                   int quad_order = 10,
                                   double time_step = 0.01);
};

export template<typename T>
concept CP = std::derived_from<T, ContinuousProcess>;

export template<CP T>
struct ContinuousTrajectory {
    double duration;
    T& process;

    ContinuousTrajectory(double duration, T& process)
        : duration(duration), process(process) {}
};

export template<typename T>
struct Moment {

};

template<CP T>
struct Moment<T> {};

auto ContinuousProcess::mean(double duration, size_t particles, double time_step) -> Result<double> {
    auto traj = ContinuousTrajectory {duration, *this};
}
