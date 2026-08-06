#include "genetic_algorithm.h"

#include <algorithm>
#include <array>
#include <stdexcept>
#include <utility>

namespace {

constexpr std::array<const char*, 7> kOperations = {
    "y = y + 1",
    "y = y - 1",
    "y = y * 2",
    "y = y / 2",
    "y = sin(y)",
    "y = cos(y)",
    "y = ln(y)",
};

}  // namespace

GeneticAlgorithm::GeneticAlgorithm(
    std::vector<double> input_values,
    std::vector<double> desired_output,
    std::size_t population_size,
    std::size_t generations,
    std::uint32_t seed)
    : input_values(std::move(input_values)),
      desired_output(std::move(desired_output)),
      population_size(population_size),
      generations(generations),
      random_engine(seed) {
    if (this->input_values.empty()) {
        throw std::invalid_argument("Input values must not be empty");
    }
    if (this->input_values.size() != this->desired_output.size()) {
        throw std::invalid_argument(
            "Input values and desired output must have the same length");
    }
    if (this->population_size < 2) {
        throw std::invalid_argument("Population size must be at least 2");
    }
}

Function GeneticAlgorithm::run() {
    generateInitialPopulation();

    for (std::size_t generation = 0; generation < generations; ++generation) {
        evaluatePopulation();
        selectBestIndividuals();
        performMutation();
    }

    evaluatePopulation();
    return getBestFunction();
}

void GeneticAlgorithm::generateInitialPopulation() {
    population.clear();
    population.reserve(population_size);
    for (std::size_t index = 0; index < population_size; ++index) {
        population.push_back(
            Function::createFromInstructions(generateRandomInstructions()));
    }
}

std::string GeneticAlgorithm::generateRandomInstructions() {
    std::uniform_int_distribution<int> step_distribution(2, 5);
    const int steps = step_distribution(random_engine);

    std::string instructions = "y = x";
    for (int index = 0; index < steps; ++index) {
        instructions += ", " + generateRandomInstruction();
    }
    return instructions;
}

void GeneticAlgorithm::evaluatePopulation() {
    for (Function& function : population) {
        const std::vector<double> results = function.calculate(input_values);
        function.evaluateSimilarity(results, desired_output);
    }
}

void GeneticAlgorithm::selectBestIndividuals() {
    std::sort(
        population.begin(),
        population.end(),
        [](const Function& left, const Function& right) {
            return left.getScore() > right.getScore();
        });

    const std::size_t survivor_count = (population.size() + 1) / 2;
    population.erase(
        population.begin() + static_cast<std::ptrdiff_t>(survivor_count),
        population.end());
}

void GeneticAlgorithm::performMutation() {
    const std::size_t survivor_count = population.size();
    std::size_t parent_index = 0;

    while (population.size() < population_size) {
        Function mutated_function = population[parent_index];
        const std::size_t mutable_instruction_count =
            mutated_function.getInstructions().size() - 1;
        std::uniform_int_distribution<std::size_t> instruction_distribution(
            1,
            mutable_instruction_count);
        mutated_function.substituteInstruction(
            instruction_distribution(random_engine),
            generateRandomInstruction());
        population.push_back(std::move(mutated_function));
        parent_index = (parent_index + 1) % survivor_count;
    }
}

std::string GeneticAlgorithm::generateRandomInstruction() {
    std::uniform_int_distribution<std::size_t> operation_distribution(
        0,
        kOperations.size() - 1);
    return kOperations[operation_distribution(random_engine)];
}

Function GeneticAlgorithm::getBestFunction() const {
    if (population.empty()) {
        throw std::logic_error("Population has not been initialized");
    }

    return *std::max_element(
        population.begin(),
        population.end(),
        [](const Function& left, const Function& right) {
            return left.getScore() < right.getScore();
        });
}
