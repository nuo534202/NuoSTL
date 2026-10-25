#include "test_harness.hpp"

#include <cstddef>
#include <functional>
#include <iostream>
#include <new>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>

#include "test_framework.hpp"

namespace test
{

static_assert(!std::is_constructible_v<TestContext, std::ostringstream&&>);
static_assert(!test::detail::BooleanCheckExpression<decltype("message")>);
static_assert(!test::detail::BooleanCheckExpression<const char*>);
static_assert(test::detail::BooleanCheckExpression<int>);

namespace
{

class TrackingStringBuffer : public std::stringbuf
{
public:
    int SyncCount() const noexcept
    {
        return sync_count_;
    }

protected:
    int sync() override
    {
        ++sync_count_;
        return std::stringbuf::sync();
    }

private:
    int sync_count_ = 0;
};

class ThrowingStringBuffer : public std::stringbuf
{
public:
    int SyncCount() const noexcept
    {
        return sync_count_;
    }

protected:
    int sync() override
    {
        ++sync_count_;
        throw std::bad_alloc();
    }

private:
    int sync_count_ = 0;
};

class ReentrantStringBuffer : public std::stringbuf
{
public:
    void SetContext(TestContext* context) noexcept
    {
        context_ = context;
    }

protected:
    int sync() override
    {
        if (context_ != nullptr && !reentered_)
        {
            reentered_ = true;
            context_->Check(false, "reentrant check", __FILE__, __LINE__);
        }
        return std::stringbuf::sync();
    }

private:
    TestContext* context_ = nullptr;
    bool reentered_ = false;
};

class CerrRedirect
{
public:
    explicit CerrRedirect(std::streambuf* buffer)
        : previous_buffer_(std::cerr.rdbuf()),
          previous_state_(std::cerr.rdstate()),
          previous_exceptions_(std::cerr.exceptions())
    {
        try
        {
            std::cerr.exceptions(std::ios::goodbit);
            std::cerr.rdbuf(buffer);
            std::cerr.clear();
        }
        catch (...)
        {
            Restore();
            throw;
        }
    }

    ~CerrRedirect() noexcept
    {
        Restore();
    }

    CerrRedirect(const CerrRedirect&) = delete;
    CerrRedirect& operator=(const CerrRedirect&) = delete;

private:
    void Restore() noexcept
    {
        std::cerr.exceptions(std::ios::goodbit);
        std::cerr.rdbuf(previous_buffer_);
        std::cerr.clear(previous_state_);
        try
        {
            std::cerr.exceptions(previous_exceptions_);
        }
        catch (...)
        {
        }
    }

    std::streambuf* previous_buffer_;
    std::ios::iostate previous_state_;
    std::ios::iostate previous_exceptions_;
};

}  /* namespace */

void TestHarness::TestSuite()
{
    if (!test::IsSuiteActive())
    {
        throw std::logic_error(
            "TestHarness::TestSuite requires an active test suite");
    }

    TestPassingSuite();
    TestFailingCheck();
    TestEmptySuite();
    TestNestedSuites();
    TestCheckDiagnostics();
    TestReportingFailureDoesNotEscape();
    TestUnexpectedExceptions();
}

void TestHarness::TestPassingSuite()
{
    std::ostringstream output;
    TestContext context(output);
    int evaluation_count = 0;

    context.RunSuite("passing suite", [&evaluation_count]()
    {
        NUO_TEST_CHECK(++evaluation_count == 1);
        NUO_TEST_CHECK(evaluation_count == 1);
    });

    NUO_TEST_CHECK(context.SuiteCount() == 1);
    NUO_TEST_CHECK(evaluation_count == 1);
    NUO_TEST_CHECK(context.FailureCount() == 0);
    NUO_TEST_CHECK(context.FailedSuiteCount() == 0);
    NUO_TEST_CHECK(!context.HasFailures());
    NUO_TEST_CHECK(output.str().empty());
}

void TestHarness::TestFailingCheck()
{
    TrackingStringBuffer buffer;
    std::ostream output(&buffer);
    TestContext context(output);

    context.RunSuite("intentional check failure", []()
    {
        NUO_TEST_CHECK(false);
    });
    context.RunSuite("subsequent passing suite", []()
    {
        NUO_TEST_CHECK(true);
    });

    NUO_TEST_CHECK(context.SuiteCount() == 2);
    NUO_TEST_CHECK(context.FailureCount() == 1);
    NUO_TEST_CHECK(context.FailedSuiteCount() == 1);
    NUO_TEST_CHECK(context.HasFailures());
    NUO_TEST_CHECK(buffer.SyncCount() > 0);
    const std::string failure_output = buffer.str();
    const std::string expected_prefix =
        "[FAIL] intentional check failure: " + std::string(__FILE__) + ":";
    const std::size_t prefix_position = failure_output.find(expected_prefix);
    NUO_TEST_CHECK(
        prefix_position != std::string::npos);
    if (prefix_position == std::string::npos)
    {
        return;
    }
    const std::size_t line_start = prefix_position + expected_prefix.size();
    const std::size_t line_end = failure_output.find(": false\n", line_start);
    NUO_TEST_CHECK(
        line_end != std::string::npos && line_end > line_start);
}

void TestHarness::TestEmptySuite()
{
    std::ostringstream output;
    TestContext context(output);
    const std::function<void()> empty_suite;

    context.RunSuite("empty callback", empty_suite);

    NUO_TEST_CHECK(context.SuiteCount() == 1);
    NUO_TEST_CHECK(context.FailureCount() == 1);
    NUO_TEST_CHECK(context.FailedSuiteCount() == 1);
    NUO_TEST_CHECK(
        output.str().find(
            "[FAIL] empty callback: empty suite callback\n") !=
        std::string::npos);
    NUO_TEST_CHECK(
        output.str().find("unexpected exception") == std::string::npos);
}

void TestHarness::TestNestedSuites()
{
    std::ostringstream output;
    TestContext context(output);

    context.RunSuite("outer suite", [&context]()
    {
        context.RunSuite("inner suite", []()
        {
            NUO_TEST_CHECK(false);
        });
        NUO_TEST_CHECK(context.FailureCount() == 1);
        NUO_TEST_CHECK(std::is_same_v<int, int>);
        NUO_TEST_CHECK(std::pair{1, 2} == std::pair{1, 2});
    });

    NUO_TEST_CHECK(context.SuiteCount() == 2);
    NUO_TEST_CHECK(context.FailureCount() == 1);
    NUO_TEST_CHECK(context.FailedSuiteCount() == 1);
    const std::string nested_output = output.str();
    NUO_TEST_CHECK(
        nested_output.find("[FAIL] inner suite:") != std::string::npos);
    NUO_TEST_CHECK(
        nested_output.find("[FAIL] outer suite:") == std::string::npos);
}

void TestHarness::TestCheckDiagnostics()
{
    std::ostringstream output;
    TestContext context(output);

    context.RunSuite("null check diagnostics", [&context]()
    {
        context.Check(false, nullptr, nullptr, 7);
    });

    NUO_TEST_CHECK(
        output.str().find("<unknown file>:7: <unknown expression>\n") !=
        std::string::npos);
    NUO_TEST_CHECK(!context.HasDetachedFailures());
    NUO_TEST_CHECK(context.FailureCount() == 1);
    NUO_TEST_CHECK(context.FailedSuiteCount() == 1);
}

void TestHarness::TestReportingFailureDoesNotEscape()
{
    std::ostringstream output;
    output.exceptions(std::ios::badbit);
    try
    {
        output.setstate(std::ios::badbit);
    }
    catch (const std::ios_base::failure&)
    {
        /* The bad stream is the expected test input. */
    }

    TestContext context(output);
    std::ostringstream reporting_fallback;
    {
        CerrRedirect redirect(reporting_fallback.rdbuf());
        context.RunSuite("reporting failure", []()
        {
            throw std::runtime_error("expected exception");
        });
    }

    NUO_TEST_CHECK(context.SuiteCount() == 1);
    NUO_TEST_CHECK(context.FailureCount() == 1);
    NUO_TEST_CHECK(context.FailedSuiteCount() == 1);
    NUO_TEST_CHECK(context.ReportingFailureCount() == 1);
    NUO_TEST_CHECK(
        reporting_fallback.str().find("unexpected exception") !=
        std::string::npos);

    ThrowingStringBuffer buffer;
    std::ostream throwing_output(&buffer);
    throwing_output.exceptions(std::ios::badbit);
    TestContext throwing_context(throwing_output);
    std::ostringstream fallback_output;
    {
        CerrRedirect redirect(fallback_output.rdbuf());
        throwing_context.RunSuite("throwing stream", []()
        {
            throw std::runtime_error("reporting exception");
        });
    }

    NUO_TEST_CHECK(buffer.SyncCount() > 0);
    NUO_TEST_CHECK(
        fallback_output.str().find("[FAIL] ") != std::string::npos);
    NUO_TEST_CHECK(
        fallback_output.str().find("throwing stream") != std::string::npos);
    NUO_TEST_CHECK(
        fallback_output.str().find("reporting exception") !=
        std::string::npos);
    NUO_TEST_CHECK(throwing_context.FailureCount() == 1);
    NUO_TEST_CHECK(throwing_context.FailedSuiteCount() == 1);
    NUO_TEST_CHECK(throwing_context.ReportingFailureCount() == 1);
    NUO_TEST_CHECK(!throwing_context.HasDetachedFailures());

    ThrowingStringBuffer secondary_buffer;
    TestContext secondary_context(throwing_output);
    {
        CerrRedirect redirect(&secondary_buffer);
        secondary_context.RunSuite("secondary reporting failure", []()
        {
            throw std::runtime_error("secondary failure");
        });
    }

    NUO_TEST_CHECK(secondary_buffer.SyncCount() > 0);
    NUO_TEST_CHECK(secondary_context.FailureCount() == 1);
    NUO_TEST_CHECK(secondary_context.ReportingFailureCount() == 2);

    ThrowingStringBuffer last_resort_buffer;
    TestContext last_resort_context;
    {
        CerrRedirect redirect(&last_resort_buffer);
        std::cerr.exceptions(std::ios::badbit);
        last_resort_context.RunSuite("last resort reporting", []()
        {
            throw std::runtime_error("last resort failure");
        });
    }

    NUO_TEST_CHECK(last_resort_buffer.SyncCount() > 0);
    NUO_TEST_CHECK(last_resort_context.FailureCount() == 1);
    NUO_TEST_CHECK(last_resort_context.ReportingFailureCount() == 1);
    NUO_TEST_CHECK(!last_resort_context.HasDetachedFailures());

    ReentrantStringBuffer reentrant_buffer;
    std::ostream reentrant_output(&reentrant_buffer);
    TestContext reentrant_context(reentrant_output);
    reentrant_buffer.SetContext(&reentrant_context);
    reentrant_context.RunSuite("reentrant reporting", []()
    {
        NUO_TEST_CHECK(false);
    });
    reentrant_buffer.SetContext(nullptr);

    NUO_TEST_CHECK(reentrant_context.FailureCount() == 2);
    NUO_TEST_CHECK(reentrant_context.FailedSuiteCount() == 1);
    NUO_TEST_CHECK(reentrant_context.ReportingFailureCount() == 1);
}

void TestHarness::TestUnexpectedExceptions()
{
    std::ostringstream output;
    TestContext context(output);

    context.RunSuite("standard exception", []()
    {
        throw std::runtime_error("expected standard exception");
    });
    context.RunSuite("unknown exception", []()
    {
        throw 42;
    });

    NUO_TEST_CHECK(context.SuiteCount() == 2);
    NUO_TEST_CHECK(context.FailureCount() == 2);
    NUO_TEST_CHECK(context.FailedSuiteCount() == 2);
    const std::string exception_output = output.str();
    NUO_TEST_CHECK(
        exception_output.find(
            "[FAIL] standard exception: unexpected exception: "
            "expected standard exception\n") !=
        std::string::npos);
    NUO_TEST_CHECK(
        exception_output.find(
            "[FAIL] unknown exception: unexpected exception: "
            "unknown exception\n") !=
        std::string::npos);
}

}  /* namespace test */
