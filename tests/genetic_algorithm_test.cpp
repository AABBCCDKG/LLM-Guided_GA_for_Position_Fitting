#include "genetic_algorithm.h"

#include "test_support.h"

#include <cmath>
#include <stdexcept>
#include <type_traits>
#include <vector>

namespace {

void testConstructorValidation(TestRunner& runner) {
    runner.expectThrows<std::invalid_argument>(
        [] { GeneticAlgorithm algorithm({}, {}, 4, 2, 1); },
        "empty input is rejected");
    runner.expectThrows<std::invalid_argument>(
        [] { GeneticAlgorithm algorithm({1.0}, {1.0, 2.0}, 4, 2, 1); },
        "mismatched input and output are rejected");
    runner.expectThrows<std::invalid_argument>(
        [] { GeneticAlgorithm algorithm({1.0}, {2.0}, 1, 2, 1); },
        "population smaller than two is rejected");
}

void testRunReturnsEvaluatedFunction(TestRunner& runner) {
    const std::vector<double> input = {1.0, 2.0, 3.0, 4.0};
    const std::vector<double> desired = {2.0, 4.0, 6.0, 8.0};
    GeneticAlgorithm algorithm(input, desired, 5, 3, 42);

    const Function best = algorithm.run();
    Function rescored = best;
    const double independently_computed_score =
        rescored.evaluateSimilarity(best.calculate(input), desired);

    runner.expect(
        best.getInstructions().size() >= 3,
        "generated functions contain mutable instructions");
    runner.expect(
        std::isfinite(best.getScore()),
        "returned score is finite");
    runner.expect(
        best.getScore() >= 0.0 && best.getScore() <= 1.0,
        "returned score is in the similarity range");
    runner.expectNear(
        best.getScore(),
        independently_computed_score,
        1e-12,
        "returned function has been evaluated after the final mutation");
}

void testSeedMakesRunReproducible(TestRunner& runner) {
    GeneticAlgorithm first({1.0, 2.0}, {2.0, 4.0}, 6, 4, 1234);
    GeneticAlgorithm second({1.0, 2.0}, {2.0, 4.0}, 6, 4, 1234);

    const Function first_result = first.run();
    const Function second_result = second.run();

    runner.expect(
        first_result.getInstructions() == second_result.getInstructions(),
        "equal seeds produce equal best instructions");
    runner.expectNear(
        first_result.getScore(),
        second_result.getScore(),
        1e-12,
        "equal seeds produce equal scores");
}

void testZeroGenerationsStillEvaluatesPopulation(TestRunner& runner) {
    GeneticAlgorithm algorithm({1.0, 2.0}, {1.0, 2.0}, 2, 0, 7);
    const Function result = algorithm.run();
    Function rescored = result;
    const double independently_computed_score =
        rescored.evaluateSimilarity(result.calculate({1.0, 2.0}), {1.0, 2.0});

    runner.expectNear(
        result.getScore(),
        independently_computed_score,
        1e-12,
        "zero-generation run evaluates its initial population");
}

}  // namespace

int main() {
    static_assert(
        !std::is_default_constructible_v<Function>,
        "Function should not need a meaningless default constructor");

    TestRunner runner;
    testConstructorValidation(runner);
    testRunReturnsEvaluatedFunction(runner);
    testSeedMakesRunReproducible(runner);
    testZeroGenerationsStillEvaluatesPopulation(runner);
    return runner.finish();
}
