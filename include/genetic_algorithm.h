#ifndef SMARTGA_GENETIC_ALGORITHM_H
#define SMARTGA_GENETIC_ALGORITHM_H

#include <cstddef>
#include <cstdint>
#include <random>
#include <string>
#include <vector>

#include "expression_function.h"

class GeneticAlgorithm {
public:
    GeneticAlgorithm(
        std::vector<double> input_values,
        std::vector<double> desired_output,
        std::size_t population_size,
        std::size_t generations,
        std::uint32_t seed = std::random_device{}());

    Function run();

private:
    std::vector<double> input_values;
    std::vector<double> desired_output;
    std::size_t population_size;
    std::size_t generations;
    std::vector<Function> population;
    std::mt19937 random_engine;

    void generateInitialPopulation();
    std::string generateRandomInstructions();
    void evaluatePopulation();
    void selectBestIndividuals();
    void performMutation();
    std::string generateRandomInstruction();
    Function getBestFunction() const;
};

#endif  // SMARTGA_GENETIC_ALGORITHM_H
