#include "test_framework.hpp"

#include <atomic>
#include <cstdio>
#include <exception>
#include <ios>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>

namespace test
{

namespace
{

thread_local TestContext* active_context = nullptr;
thread_local bool reporting_failure_in_progress = false;

std::atomic<std::size_t>& ProcessFailureCounter()
{
    static std::atomic<std::size_t> count{0};
    return count;
}

std::recursive_mutex& ReportingMutex()
{
    static std::recursive_mutex mutex;
    return mutex;
}

class ReportingScope
{
public:
    ReportingScope()
    {
        reporting_failure_in_progress = true;
    }

    ~ReportingScope()
    {
        reporting_failure_in_progress = false;
    }

    ReportingScope(const ReportingScope&) = delete;
    ReportingScope& operator=(const ReportingScope&) = delete;
};

bool WriteFailureReport(
    std::ostream& output,
    std::string_view message) noexcept
{
    try
    {
        const std::lock_guard<std::recursive_mutex> lock(
            ReportingMutex());
        output.write(
            message.data(), static_cast<std::streamsize>(message.size()));
        output.flush();
        return output.good();
    }
    catch (...)
    {
        return false;
    }
}

}  /* namespace */

class TestContext::SuiteFrameScope
{
public:
    SuiteFrameScope(TestContext& context, const char* suite_name)
        : context_(context),
          frame_{suite_name == nullptr ? "" : suite_name, 0,
                 context.suite_frame_}
    {
        context_.suite_frame_ = &frame_;
    }

    ~SuiteFrameScope()
    {
        context_.suite_frame_ = frame_.previous;
    }

    std::size_t FailureCount() const noexcept
    {
        return frame_.failure_count;
    }

    SuiteFrameScope(const SuiteFrameScope&) = delete;
    SuiteFrameScope& operator=(const SuiteFrameScope&) = delete;

private:
    TestContext& context_;
    SuiteFrame frame_;
};

class TestContext::SuiteScope
{
public:
    explicit SuiteScope(TestContext& context)
        : previous_context_(active_context)
    {
        active_context = &context;
    }

    ~SuiteScope()
    {
        active_context = previous_context_;
    }

    SuiteScope(const SuiteScope&) = delete;
    SuiteScope& operator=(const SuiteScope&) = delete;

private:
    TestContext* previous_context_;
};

TestContext::TestContext()
    : output_(nullptr)
{
}

TestContext::TestContext(std::ostream& output)
    : output_(&output)
{
}

void TestContext::RunSuite(
    const char* suite_name,
    const std::function<void()>& suite)
{
    SuiteScope context_scope(*this);
    SuiteFrameScope suite_scope(*this, suite_name);
    ++suite_count_;

    try
    {
        if (!suite)
        {
            ReportFailure("empty suite callback");
        }
        else
        {
            suite();
        }
    }
    catch (const std::exception& exception)
    {
        RecordUnexpectedException(exception.what());
    }
    catch (...)
    {
        RecordUnexpectedException("unknown exception");
    }

    if (suite_scope.FailureCount() != 0)
    {
        ++failed_suite_count_;
    }
}

void TestContext::Check(
    bool condition,
    const char* expression,
    const char* file,
    int line)
{
    if (condition)
    {
        return;
    }

    try
    {
        std::ostringstream message;
        message << (file == nullptr ? "<unknown file>" : file)
                << ':' << line << ": "
                << (expression == nullptr
                        ? "<unknown expression>"
                        : expression);
        ReportFailure(message.str());
    }
    catch (...)
    {
        char fallback[256];
        std::snprintf(
            fallback,
            sizeof(fallback),
            "%s:%d: check failed (diagnostic unavailable)",
            file == nullptr ? "<unknown file>" : file,
            line);
        ReportFailure(fallback);
    }
}

std::size_t TestContext::FailureCount() const noexcept
{
    return failure_count_;
}

std::size_t TestContext::SuiteCount() const noexcept
{
    return suite_count_;
}

std::size_t TestContext::FailedSuiteCount() const noexcept
{
    return failed_suite_count_;
}

bool TestContext::HasFailures() const noexcept
{
    return failure_count_ != 0;
}

bool TestContext::HasDetachedFailures() const noexcept
{
    return detached_failure_count_ != 0;
}

std::size_t TestContext::ReportingFailureCount() const noexcept
{
    return reporting_failure_count_;
}

void TestContext::RecordUnexpectedException(const char* message) noexcept
{
    try
    {
        std::string description = "unexpected exception: ";
        description += message == nullptr ? "unknown exception" : message;
        ReportFailure(description);
    }
    catch (...)
    {
        ReportFailure("unexpected exception");
    }
}

void TestContext::ReportFailure(std::string_view message) noexcept
{
    ++failure_count_;
    if (active_context == this)
    {
        if (suite_frame_ != nullptr)
        {
            ++suite_frame_->failure_count;
        }
    }
    else
    {
        ++detached_failure_count_;
        ProcessFailureCounter().fetch_add(1, std::memory_order_release);
    }

    if (reporting_failure_in_progress)
    {
        ++reporting_failure_count_;
        return;
    }

    const ReportingScope reporting_scope;
    std::string formatted;
    try
    {
        std::ostringstream stream;
        stream << "[FAIL] ";
        if (suite_frame_ != nullptr)
        {
            stream << suite_frame_->name << ": ";
        }
        stream << message << '\n';
        formatted = stream.str();
    }
    catch (...)
    {
        ++reporting_failure_count_;
        constexpr std::string_view kFallbackMessage =
            "[FAIL] failure reporting unavailable\n";
        if (!WriteFailureReport(std::cerr, kFallbackMessage))
        {
            ++reporting_failure_count_;
        }
        return;
    }

    std::ostream* configured_output = output_ == nullptr ? &std::cerr : output_;
    if (WriteFailureReport(*configured_output, formatted))
    {
        return;
    }

    ++reporting_failure_count_;
    if (configured_output == &std::cerr)
    {
        return;
    }

    if (!WriteFailureReport(std::cerr, formatted))
    {
        ++reporting_failure_count_;
    }
}

TestContext& CurrentTestContext()
{
    if (active_context != nullptr)
    {
        return *active_context;
    }

    static thread_local TestContext fallback_context;
    return fallback_context;
}

bool IsSuiteActive() noexcept
{
    return active_context != nullptr;
}

std::size_t ProcessFailureCount() noexcept
{
    return ProcessFailureCounter().load(std::memory_order_acquire);
}

bool HasProcessFailuresSince(std::size_t baseline) noexcept
{
    return ProcessFailureCount() > baseline;
}

}  /* namespace test */
