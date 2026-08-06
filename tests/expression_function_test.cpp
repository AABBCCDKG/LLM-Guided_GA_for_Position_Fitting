#include "expression_function.h"

#include "test_support.h"

#include <cmath>
#include <stdexcept>
#include <vector>

namespace {

void testParsingAndCalculation(TestRunner& runner) {
    Function function = Function::createFromInstructions(
        " y=x , y=y+5 , y = sin ( y ) ");

    runner.expect(
        function.getInstructions() ==
            std::vector<std::string>({"y = x", "y = y + 5", "y = sin(y)"}),
        "instructions are normalized");
    runner.expect(
        function.getExpression() == "sin((x + 5))",
        "expression reflects normalized instructions");

    const std::vector<double> values = function.calculate({1.0, 2.0});
    runner.expect(values.size() == 2, "calculation preserves input size");
    runner.expectNear(values[0], std::sin(6.0), 1e-12, "first result is correct");
    runner.expectNear(values[1], std::sin(7.0), 1e-12, "second result is correct");
}

void testArithmeticInstructions(TestRunner& runner) {
    const Function function = Function::createFromInstructions(
        "y = x, y = y + 2, y = y * 3, y = y - 2, y = y / 5, y = y ^ 3");
    const std::vector<double> values = function.calculate({2.0});

    runner.expectNear(values.front(), 8.0, 1e-12, "arithmetic chain is correct");
    runner.expect(
        function.getExpression() == "(((((x + 2) * 3) - 2) / 5))^3",
        "arithmetic expression is updated");
}

void testScientificNotationAndCompleteParsing(TestRunner& runner) {
    const Function function =
        Function::createFromInstructions("y = x, y = y + 1e-3");
    const std::vector<double> values = function.calculate({2.0});

    runner.expectNear(
        values.front(),
        2.001,
        1e-12,
        "scientific notation is parsed as one numeric operand");
    runner.expect(
        function.getInstructions()[1] == "y = y + 1e-3",
        "scientific notation keeps its exponent sign");
    runner.expectThrows<std::invalid_argument>(
        [] { Function::createFromInstructions("y = x, y = y + 1oops"); },
        "numeric operands reject trailing garbage");
}

void testSimilarityAndMutation(TestRunner& runner) {
    Function function =
        Function::createFromInstructions("y = x, y = y + 1");

    runner.expectNear(
        function.evaluateSimilarity({2.0, 3.0}, {2.0, 3.0}),
        1.0,
        1e-12,
        "identical values receive a perfect score");
    function.substituteInstruction(1, " y=y*2 ");
    runner.expectNear(
        function.getScore(),
        0.0,
        1e-12,
        "mutation invalidates the previous score");
    runner.expect(
        function.getExpression() == "(x * 2)",
        "mutation updates the expression");
    runner.expect(
        function.getInstructions()[1] == "y = y * 2",
        "mutation normalizes the replacement");
}

void testMutationStrongExceptionSafety(TestRunner& runner) {
    Function function =
        Function::createFromInstructions("y = x, y = y + 1");
    function.evaluateSimilarity({2.0, 3.0}, {2.0, 3.0});

    const std::vector<std::string> original_instructions =
        function.getInstructions();
    const std::string original_expression = function.getExpression();
    const double original_score = function.getScore();

    runner.expectThrows<std::invalid_argument>(
        [&function] { function.substituteInstruction(1, "y = y + 1oops"); },
        "invalid replacement is rejected before mutation is committed");
    runner.expect(
        function.getInstructions() == original_instructions,
        "failed mutation preserves the original instructions");
    runner.expect(
        function.getExpression() == original_expression,
        "failed mutation preserves the original expression");
    runner.expectNear(
        function.getScore(),
        original_score,
        1e-12,
        "failed mutation preserves the original score");
}

void testInvalidInputs(TestRunner& runner) {
    runner.expectThrows<std::invalid_argument>(
        [] { Function::createFromInstructions("y = y + 1"); },
        "instruction list must begin with y = x");
    runner.expectThrows<std::invalid_argument>(
        [] {
            Function function =
                Function::createFromInstructions("y = x, y = tangent(y)");
            function.calculate({1.0});
        },
        "unsupported instructions are rejected");
    runner.expectThrows<std::domain_error>(
        [] {
            const Function function =
                Function::createFromInstructions("y = x, y = y / 0");
            function.calculate({1.0});
        },
        "division by zero is rejected");
    runner.expectThrows<std::invalid_argument>(
        [] {
            Function function = Function::createFromInstructions("y = x");
            function.evaluateSimilarity({}, {});
        },
        "empty samples are rejected");
    runner.expectThrows<std::invalid_argument>(
        [] {
            Function function = Function::createFromInstructions("y = x");
            function.evaluateSimilarity({1.0}, {1.0, 2.0});
        },
        "mismatched samples are rejected");
    runner.expectThrows<std::out_of_range>(
        [] {
            Function function =
                Function::createFromInstructions("y = x, y = y + 1");
            function.substituteInstruction(0, "y = y - 1");
        },
        "the immutable first instruction cannot be replaced");
}

void testNonFiniteCalculationScoresZero(TestRunner& runner) {
    Function function =
        Function::createFromInstructions("y = x, y = ln(y)");
    const std::vector<double> calculated = function.calculate({-1.0});

    runner.expect(std::isnan(calculated.front()), "invalid logarithm produces NaN");
    runner.expectNear(
        function.evaluateSimilarity(calculated, {0.0}),
        0.0,
        1e-12,
        "non-finite output receives the lowest score");
}

}  // namespace

int main() {
    TestRunner runner;
    testParsingAndCalculation(runner);
    testArithmeticInstructions(runner);
    testScientificNotationAndCompleteParsing(runner);
    testSimilarityAndMutation(runner);
    testMutationStrongExceptionSafety(runner);
    testInvalidInputs(runner);
    testNonFiniteCalculationScoresZero(runner);
    return runner.finish();
}
