/* Generated file, do not edit */

#ifndef CXXTEST_RUNNING
#define CXXTEST_RUNNING
#endif

#define _CXXTEST_HAVE_STD
#include "cxxtest/cxxtest/TestListener.h"
#include "cxxtest/cxxtest/TestTracker.h"
#include "cxxtest/cxxtest/TestRunner.h"
#include "cxxtest/cxxtest/RealDescriptions.h"
#include "cxxtest/cxxtest/TestMain.h"
#include "cxxtest/cxxtest/ErrorPrinter.h"

int main( int argc, char *argv[] ) {
 int status;
    CxxTest::ErrorPrinter tmp;
    CxxTest::RealWorldDescription::_worldName = "cxxtest";
    status = CxxTest::Main< CxxTest::ErrorPrinter >( tmp, argc, argv );
    return status;
}
bool suite_Tests_init = false;
#include "test_suite.h"

static Tests suite_Tests;

static CxxTest::List Tests_Tests = { 0, 0 };
CxxTest::StaticSuiteDescription suiteDescription_Tests( "test_suite.h", 18, "Tests", suite_Tests, Tests_Tests );

static class TestDescription_suite_Tests_test_bot : public CxxTest::RealTestDescription {
public:
 TestDescription_suite_Tests_test_bot() : CxxTest::RealTestDescription( Tests_Tests, suiteDescription_Tests, 20, "test_bot" ) {}
 void runTest() { suite_Tests.test_bot(); }
} testDescription_suite_Tests_test_bot;

static class TestDescription_suite_Tests_test_state : public CxxTest::RealTestDescription {
public:
 TestDescription_suite_Tests_test_state() : CxxTest::RealTestDescription( Tests_Tests, suiteDescription_Tests, 23, "test_state" ) {}
 void runTest() { suite_Tests.test_state(); }
} testDescription_suite_Tests_test_state;

static class TestDescription_suite_Tests_test_utils : public CxxTest::RealTestDescription {
public:
 TestDescription_suite_Tests_test_utils() : CxxTest::RealTestDescription( Tests_Tests, suiteDescription_Tests, 26, "test_utils" ) {}
 void runTest() { suite_Tests.test_utils(); }
} testDescription_suite_Tests_test_utils;

static class TestDescription_suite_Tests_test_network : public CxxTest::RealTestDescription {
public:
 TestDescription_suite_Tests_test_network() : CxxTest::RealTestDescription( Tests_Tests, suiteDescription_Tests, 29, "test_network" ) {}
 void runTest() { suite_Tests.test_network(); }
} testDescription_suite_Tests_test_network;

static class TestDescription_suite_Tests_test_agent : public CxxTest::RealTestDescription {
public:
 TestDescription_suite_Tests_test_agent() : CxxTest::RealTestDescription( Tests_Tests, suiteDescription_Tests, 32, "test_agent" ) {}
 void runTest() { suite_Tests.test_agent(); }
} testDescription_suite_Tests_test_agent;

#include "cxxtest/cxxtest/Root.cpp"
const char* CxxTest::RealWorldDescription::_worldName = "cxxtest";
