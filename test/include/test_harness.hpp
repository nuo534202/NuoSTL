#pragma once

namespace test
{

class TestHarness
{
public:
    /* Must be called from within an active test suite. */
    static void TestSuite();

private:
    static void TestPassingSuite();
    static void TestFailingCheck();
    static void TestEmptySuite();
    static void TestNestedSuites();
    static void TestCheckDiagnostics();
    static void TestReportingFailureDoesNotEscape();
    static void TestUnexpectedExceptions();
};

}  /* namespace test */
