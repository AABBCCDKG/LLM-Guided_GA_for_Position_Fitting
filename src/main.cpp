#include "genetic_algorithm.h"

#include <iostream>
#include <vector>

int main() {
    try {
        const std::vector<double> input_values = {1, 2, 3, 4, 5};
        const std::vector<double> desired_output = {2, 4, 6, 8, 10};
        GeneticAlgorithm algorithm(input_values, desired_output, 20, 25, 42);
        const Function best_function = algorithm.run();

        std::cout << "Best expression: " << best_function.getExpression() << '\n';
        std::cout << "Similarity score: " << best_function.getScore() << '\n';
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
        return 1;
    }

    return 0;
}
