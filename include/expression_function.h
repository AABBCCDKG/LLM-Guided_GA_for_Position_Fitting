#ifndef SMARTGA_EXPRESSION_FUNCTION_H
#define SMARTGA_EXPRESSION_FUNCTION_H

#include <cstddef>
#include <string>
#include <vector>

class Function {
public:
    explicit Function(std::vector<std::string> instructions);

    static Function createFromInstructions(const std::string& instruction_string);

    std::vector<double> calculate(const std::vector<double>& x_values) const;

    double evaluateSimilarity(const std::vector<double>& calculated_values, const std::vector<double>& desired_output);

    void substituteInstruction(std::size_t index, const std::string& new_instruction);

    double getScore() const;

    const std::string& getExpression() const;

    const std::vector<std::string>& getInstructions() const;

private:
    std::vector<std::string> instructions;
    std::string expression;
    double score = 0.0;

    static std::string formatInstruction(const std::string& instruction);
    void updateExpression();
};

#endif  // SMARTGA_EXPRESSION_FUNCTION_H
