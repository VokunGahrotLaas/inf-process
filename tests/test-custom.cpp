#include <inf/tests.hpp>

INF_CUST_TEST_SUITE(CustomTests)
{
public:
	using inf::unit_test::unit_test;

protected:
	void before() override { vec_ = { 0, 1, 2, 3, 4 }; }

	void after() override { vec_.clear(); }

	std::vector<int> vec_;
};

INF_CUST_UNIT_TEST(CustomTests, TestSize) { INF_ASSERT(vec_.size() == 5); }

INF_CUST_UNIT_TEST(CustomTests, TestTrue) { INF_ASSERT(true); }

INF_RUN_TESTS()
