#include <genetic_algorithm.h>

#include <cmath>
#include <vector>

int main() {
    const std::vector<double> inputs = {1.0, 2.0, 3.0};
    const std::vector<double> desired_outputs = {2.0, 4.0, 6.0};
    GeneticAlgorithm algorithm(inputs, desired_outputs, 10, 5, 42);
    const Function result = algorithm.run();

    const std::vector<double> calculated = result.calculate(inputs);
    if (calculated.size() != inputs.size() ||
        !std::isfinite(result.getScore())) {
        return 1;
    }

    return 0;
}
