// inf
#include <inf/stdio_stream.hpp>
#include <inf/tests.hpp>

INF_TEST_SUITE(TestSuite)

INF_UNIT_TEST(TestSuite, UnitTestFalse) { INF_ASSERT(false); }

INF_UNIT_TEST(TestSuite, UnitTestTwoFalse)
{
	INF_EXPECT(false);
	INF_EXPECT(false);
	INF_ASSERT(false);
	INF_EXPECT(false);
}

INF_UNIT_TEST(TestSuite, UnitTestTrue) { INF_ASSERT(true); }

INF_RUN_TESTS()
