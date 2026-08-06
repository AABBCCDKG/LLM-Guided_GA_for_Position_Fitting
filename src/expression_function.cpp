#include "expression_function.h"

#include <cmath>
#include <limits>
#include <regex>
#include <sstream>
#include <stdexcept>
#include <utility>

namespace {

std::string operandText(const std::string& instruction, char operation) {
    const std::size_t position = instruction.find(operation);
    if (position == std::string::npos) {
        throw std::invalid_argument("Invalid instruction: " + instruction);
    }

    const std::size_t operand_start =
        instruction.find_first_not_of(' ', position + 1);
    if (operand_start == std::string::npos) {
        throw std::invalid_argument("Missing numeric operand: " + instruction);
    }
    return instruction.substr(operand_start);
}

double parseOperand(const std::string& instruction, char operation) {
    const std::string operand = operandText(instruction, operation);
    std::size_t consumed = 0;
    double value = 0.0;
    try {
        value = std::stod(operand, &consumed);
    } catch (const std::invalid_argument&) {
        throw std::invalid_argument("Invalid numeric operand: " + operand);
    }

    if (consumed != operand.size()) {
        throw std::invalid_argument("Invalid numeric operand: " + operand);
    }
    return value;
}

std::string buildExpression(const std::vector<std::string>& instructions) {
    std::string updated_expression = "x";
    for (std::size_t index = 1; index < instructions.size(); ++index) {
        const std::string& instruction = instructions[index];
        if (instruction.starts_with("y = y + ")) {
            parseOperand(instruction, '+');
            updated_expression =
                "(" + updated_expression + " + " + operandText(instruction, '+') + ")";
        } else if (instruction.starts_with("y = y - ")) {
            parseOperand(instruction, '-');
            updated_expression =
                "(" + updated_expression + " - " + operandText(instruction, '-') + ")";
        } else if (instruction.starts_with("y = y * ")) {
            parseOperand(instruction, '*');
            updated_expression =
                "(" + updated_expression + " * " + operandText(instruction, '*') + ")";
        } else if (instruction.starts_with("y = y / ")) {
            parseOperand(instruction, '/');
            updated_expression =
                "(" + updated_expression + " / " + operandText(instruction, '/') + ")";
        } else if (instruction.starts_with("y = y ^ ")) {
            parseOperand(instruction, '^');
            updated_expression =
                "(" + updated_expression + ")^" + operandText(instruction, '^');
        } else if (instruction == "y = ln(y)") {
            updated_expression = "ln(" + updated_expression + ")";
        } else if (instruction == "y = sin(y)") {
            updated_expression = "sin(" + updated_expression + ")";
        } else if (instruction == "y = cos(y)") {
            updated_expression = "cos(" + updated_expression + ")";
        } else {
            throw std::invalid_argument("Unsupported instruction: " + instruction);
        }
    }
    return updated_expression;
}

}  // namespace

Function::Function(std::vector<std::string> instructions)
    : instructions(std::move(instructions)) {
    updateExpression();
}

Function Function::createFromInstructions(const std::string& instruction_string) {
    std::istringstream instruction_stream(instruction_string);
    std::vector<std::string> parsed_instructions;
    std::string instruction;

    while (std::getline(instruction_stream, instruction, ',')) {
        parsed_instructions.push_back(formatInstruction(instruction));
    }

    if (parsed_instructions.empty() || parsed_instructions.front() != "y = x") {
        throw std::invalid_argument("Instructions must start with 'y = x'");
    }

    return Function(std::move(parsed_instructions));
}

std::vector<double> Function::calculate(const std::vector<double>& x_values) const {
    std::vector<double> y_values;
    y_values.reserve(x_values.size());

    for (double x : x_values) {
        double y = x;
        for (std::size_t index = 1; index < instructions.size(); ++index) {
            const std::string& instruction = instructions[index];
            if (instruction.starts_with("y = y + ")) {
                y += parseOperand(instruction, '+');
            } else if (instruction.starts_with("y = y - ")) {
                y -= parseOperand(instruction, '-');
            } else if (instruction.starts_with("y = y * ")) {
                y *= parseOperand(instruction, '*');
            } else if (instruction.starts_with("y = y / ")) {
                const double divisor = parseOperand(instruction, '/');
                if (divisor == 0.0) {
                    throw std::domain_error("Division by zero");
                }
                y /= divisor;
            } else if (instruction.starts_with("y = y ^ ")) {
                y = std::pow(y, parseOperand(instruction, '^'));
            } else if (instruction == "y = ln(y)") {
                if (y <= 0.0) {
                    y = std::numeric_limits<double>::quiet_NaN();
                } else {
                    y = std::log(y);
                }
            } else if (instruction == "y = sin(y)") {
                y = std::sin(y);
            } else if (instruction == "y = cos(y)") {
                y = std::cos(y);
            } else {
                throw std::invalid_argument("Unsupported instruction: " + instruction);
            }
        }
        y_values.push_back(y);
    }

    return y_values;
}

double Function::evaluateSimilarity(
    const std::vector<double>& calculated_values,
    const std::vector<double>& desired_output) {
    if (calculated_values.size() != desired_output.size()) {
        throw std::invalid_argument(
            "Calculated values and desired output must have the same length");
    }
    if (calculated_values.empty()) {
        throw std::invalid_argument("Similarity requires at least one value");
    }

    double squared_difference_sum = 0.0;
    for (std::size_t index = 0; index < calculated_values.size(); ++index) {
        if (!std::isfinite(calculated_values[index]) ||
            !std::isfinite(desired_output[index])) {
            score = 0.0;
            return score;
        }
        const double difference = calculated_values[index] - desired_output[index];
        squared_difference_sum += difference * difference;
    }

    const double root_mean_squared_error =
        std::sqrt(squared_difference_sum / calculated_values.size());
    score = 1.0 / (1.0 + root_mean_squared_error);
    return score;
}

void Function::substituteInstruction(
    std::size_t index,
    const std::string& new_instruction) {
    if (index < 1 || index >= instructions.size()) {
        throw std::out_of_range("Instruction index out of range");
    }

    std::vector<std::string> updated_instructions = instructions;
    updated_instructions[index] = formatInstruction(new_instruction);
    std::string updated_expression = buildExpression(updated_instructions);

    instructions.swap(updated_instructions);
    expression.swap(updated_expression);
    score = 0.0;
}

double Function::getScore() const {
    return score;
}

const std::string& Function::getExpression() const {
    return expression;
}

const std::vector<std::string>& Function::getInstructions() const {
    return instructions;
}

std::string Function::formatInstruction(const std::string& instruction) {
    static const std::regex initial_assignment(
        R"(^\s*y\s*=\s*x\s*$)");
    static const std::regex arithmetic_instruction(
        R"(^\s*y\s*=\s*y\s*([+\-*/^])\s*(.*?)\s*$)");
    static const std::regex function_instruction(
        R"(^\s*y\s*=\s*(ln|sin|cos)\s*\(\s*y\s*\)\s*$)");
    static const std::regex leading_or_trailing_space(R"(^\s+|\s+$)");

    std::smatch match;
    if (std::regex_match(instruction, initial_assignment)) {
        return "y = x";
    }
    if (std::regex_match(instruction, match, arithmetic_instruction)) {
        return "y = y " + match[1].str() + " " + match[2].str();
    }
    if (std::regex_match(instruction, match, function_instruction)) {
        return "y = " + match[1].str() + "(y)";
    }
    return std::regex_replace(instruction, leading_or_trailing_space, "");
}

void Function::updateExpression() {
    std::string updated_expression = buildExpression(instructions);
    expression.swap(updated_expression);
}
