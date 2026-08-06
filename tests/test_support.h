#ifndef SMARTGA_TEST_SUPPORT_H
#define SMARTGA_TEST_SUPPORT_H

#include <cmath>
#include <exception>
#include <iostream>
#include <string>

class TestRunner {
public:
    void expect(bool condition, const std::string& message) {
        if (!condition) {
            ++failure_count;
            std::cerr << "FAILED: " << message << '\n';
        }
    }

    void expectNear(
        double actual,
        double expected,
        double tolerance,
        const std::string& message) {
        expect(
            std::abs(actual - expected) <= tolerance,
            message + " (actual=" + std::to_string(actual) +
                ", expected=" + std::to_string(expected) + ")");
    }

    template <typename Exception, typename Callable>
    void expectThrows(Callable&& callable, const std::string& message) {
        try {
            callable();
            expect(false, message + " (no exception)");
        } catch (const Exception&) {
        } catch (const std::exception& exception) {
            expect(
                false,
                message + " (wrong exception: " + exception.what() + ")");
        } catch (...) {
            expect(false, message + " (wrong non-standard exception)");
        }
    }

    int finish() const {
        if (failure_count == 0) {
            std::cout << "All checks passed\n";
        }
        return failure_count == 0 ? 0 : 1;
    }

private:
    int failure_count = 0;
};

#endif
