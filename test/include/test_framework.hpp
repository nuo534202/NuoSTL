#pragma once

#include <cstddef>
#include <functional>
#include <iosfwd>
#include <string_view>
#include <type_traits>
#include <utility>

namespace test
{

class TestContext
{
public:
    /* A context must be used by one thread at a time. */
    TestContext();

    /* The referenced stream must outlive this context. */
    explicit TestContext(std::ostream& output);
    TestContext(std::ostream&&) = delete;

    TestContext(const TestContext&) = delete;
    TestContext& operator=(const TestContext&) = delete;

    void RunSuite(const char* suite_name, const std::function<void()>& suite);
    void Check(
        bool condition,
        const char* expression,
        const char* file,
        int line);

    std::size_t FailureCount() const noexcept;
    std::size_t SuiteCount() const noexcept;
    std::size_t FailedSuiteCount() const noexcept;
    /* True if this context recorded attached or detached failures. */
    bool HasFailures() const noexcept;
    /* True if this context recorded failures while it was not active. */
    bool HasDetachedFailures() const noexcept;
    /* Counts reporting-path failures and nested reports suppressed there. */
    std::size_t ReportingFailureCount() const noexcept;

private:
    struct SuiteFrame
    {
        const char* name = "";
        std::size_t failure_count = 0;
        SuiteFrame* previous = nullptr;
    };

    class SuiteFrameScope;
    class SuiteScope;

    void ReportFailure(std::string_view message) noexcept;
    void RecordUnexpectedException(const char* message) noexcept;

    std::ostream* output_;
    SuiteFrame* suite_frame_ = nullptr;
    std::size_t failure_count_ = 0;
    std::size_t suite_count_ = 0;
    std::size_t failed_suite_count_ = 0;
    std::size_t detached_failure_count_ = 0;
    std::size_t reporting_failure_count_ = 0;
};

TestContext& CurrentTestContext();
bool IsSuiteActive() noexcept;

/* Monotonic process-wide count of detached failures. */
std::size_t ProcessFailureCount() noexcept;
/* True if a detached failure occurred after the supplied baseline. */
bool HasProcessFailuresSince(std::size_t baseline) noexcept;

namespace detail
{

/* Forward one expression while rejecting an accidental message argument. */
template <typename T>
concept BooleanCheckExpression =
    !std::is_array_v<std::remove_cvref_t<T>> &&
    !std::is_pointer_v<std::remove_cvref_t<T>> &&
    requires(T&& value)
{
    static_cast<bool>(std::forward<T>(value));
};

template <BooleanCheckExpression T>
bool ToBool(T&& value)
{
    return static_cast<bool>(std::forward<T>(value));
}

}  /* namespace detail */

}  /* namespace test */

#define NUO_TEST_CHECK(...) \
    ::test::CurrentTestContext().Check( \
        ::test::detail::ToBool(__VA_ARGS__), \
        #__VA_ARGS__, __FILE__, __LINE__)
