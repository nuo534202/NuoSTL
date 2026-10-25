#include <cstddef>
#include <iostream>
#include <source_location>
#include <sstream>
#include <string>

#include "test_framework.hpp"

#ifndef NUO_DETACHED_CHECK_EXPRESSION
#error NUO_DETACHED_CHECK_EXPRESSION must name the detached check
#endif

#ifndef NUO_DETACHED_FAILURE_COUNT
#error NUO_DETACHED_FAILURE_COUNT must provide the expected count
#endif

#ifndef NUO_PROBE_ENCLOSING_SUITE_NAME
#error NUO_PROBE_ENCLOSING_SUITE_NAME must name the enclosing suite
#endif

namespace
{

constexpr int kExpectedFailureStatus = 1;
constexpr int kProbeFailureStatus = 2;
constexpr int kDetachedFailureCount = NUO_DETACHED_FAILURE_COUNT;

int FailProbe(const char* reason)
{
    std::cerr << "[PROBE] " << reason << '\n';
    return kProbeFailureStatus;
}

void RunDetachedCheck(
    test::TestContext& context,
    std::source_location location = std::source_location::current())
{
    constexpr const char* kExpression = NUO_DETACHED_CHECK_EXPRESSION;
    const int line = static_cast<int>(location.line());
    context.Check(false, kExpression, location.file_name(), line);
}

}  /* namespace */

int main()
{
    const std::size_t failure_count_before = test::ProcessFailureCount();
    std::ostringstream outer_output;
    test::TestContext outer_context(outer_output);
    std::ostringstream detached_output;
    bool detached_detected = true;
    bool detached_context_counted = true;
    outer_context.RunSuite(
        NUO_PROBE_ENCLOSING_SUITE_NAME,
        [&detached_detected, &detached_context_counted, &detached_output]()
    {
        test::TestContext first_context(detached_output);
        RunDetachedCheck(first_context);
        detached_detected =
            detached_detected && first_context.HasDetachedFailures();
        detached_context_counted =
            detached_context_counted &&
            first_context.FailureCount() == 1 &&
            first_context.FailedSuiteCount() == 0 &&
            first_context.HasFailures();

        test::TestContext second_context(detached_output);
        RunDetachedCheck(second_context);
        detached_detected =
            detached_detected && second_context.HasDetachedFailures();
        detached_context_counted =
            detached_context_counted &&
            second_context.FailureCount() == 1 &&
            second_context.FailedSuiteCount() == 0 &&
            second_context.HasFailures();
    });

    std::ostringstream no_suite_output;
    test::TestContext no_suite_context(no_suite_output);
    RunDetachedCheck(no_suite_context);
    detached_detected =
        detached_detected && no_suite_context.HasDetachedFailures();
    detached_context_counted =
        detached_context_counted &&
        no_suite_context.FailureCount() == 1 &&
        no_suite_context.FailedSuiteCount() == 0 &&
        no_suite_context.HasFailures();
    std::cout << detached_output.str() << no_suite_output.str();

    const std::size_t failure_count_after = test::ProcessFailureCount();
    const bool process_failure_detected =
        failure_count_after > failure_count_before &&
        failure_count_after - failure_count_before ==
            static_cast<std::size_t>(kDetachedFailureCount) &&
        test::HasProcessFailuresSince(failure_count_before);
    const bool outer_context_unpolluted =
        outer_context.SuiteCount() == 1 &&
        outer_context.FailureCount() == 0 &&
        outer_context.FailedSuiteCount() == 0 &&
        !outer_context.HasFailures() &&
        !outer_context.HasDetachedFailures() &&
        outer_output.str().empty();
    const std::size_t attached_failure_baseline =
        test::ProcessFailureCount();
    std::ostringstream attached_output;
    test::TestContext attached_context(attached_output);
    attached_context.RunSuite("attached suite", [&attached_context]()
    {
        attached_context.Check(false, "attached check", __FILE__, __LINE__);
    });
    const bool attached_failure_isolated =
        test::ProcessFailureCount() == attached_failure_baseline &&
        attached_context.FailureCount() == 1 &&
        attached_context.FailedSuiteCount() == 1 &&
        !attached_context.HasDetachedFailures() &&
        attached_output.str().find("attached check") != std::string::npos;
    const std::size_t subsequent_baseline = test::ProcessFailureCount();
    test::TestContext subsequent_context;
    subsequent_context.RunSuite("subsequent suite", []()
    {
    });
    const bool subsequent_run_clean =
        !test::HasProcessFailuresSince(subsequent_baseline) &&
        !subsequent_context.HasFailures();
    if (!detached_detected)
    {
        return FailProbe("detached context did not record its failure");
    }
    if (!process_failure_detected)
    {
        return FailProbe("process failure count has an unexpected delta");
    }
    if (!outer_context_unpolluted)
    {
        return FailProbe("detached failure polluted the enclosing context");
    }
    if (!subsequent_run_clean)
    {
        return FailProbe("subsequent run observed a historical failure");
    }
    if (!detached_context_counted)
    {
        return FailProbe("detached context did not record its own counters");
    }
    if (!attached_failure_isolated)
    {
        return FailProbe(
            "attached failure advanced the detached failure counter");
    }

    /* A correct probe exits like the failing run that it verifies. */
    return kExpectedFailureStatus;
}
