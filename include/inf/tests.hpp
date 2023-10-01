#pragma once

// STL
#include <functional>
#include <future>
#include <iterator>
#include <span>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
// Unix / Windows
#ifndef _WIN32
#	include <unistd.h>
#else
#	include <process.h>
#endif
// inf
#include <inf/exceptions.hpp>
#include <inf/ioutils.hpp>
#include <inf/source_location.hpp>
#include <inf/stdio_stream.hpp>
#include <inf/utils.hpp>

#define INF_TEST_SUITE(Name)                                                                                           \
	namespace inf::test_suites_obj                                                                                     \
	{                                                                                                                  \
	static auto& Name = ::inf::make_test_suite(INF_STRX(Name));                                                        \
	}

#define INF_UNIT_TEST(TestSuite, Name)                                                                                 \
	namespace inf::unit_tests_cls::TestSuite                                                                           \
	{                                                                                                                  \
	class Name : public ::inf::unit_test                                                                               \
	{                                                                                                                  \
	public:                                                                                                            \
		Name(std::string_view ts, std::string_view name)                                                               \
			: unit_test{ ts, name }                                                                                    \
		{}                                                                                                             \
		Name(const Name&) = delete;                                                                                    \
		Name(Name&&) = delete;                                                                                         \
		Name& operator=(const Name&) = delete;                                                                         \
		Name& operator=(Name&&) = delete;                                                                              \
                                                                                                                       \
	protected:                                                                                                         \
		void main() override;                                                                                          \
	};                                                                                                                 \
	}                                                                                                                  \
	namespace inf::unit_tests_obj::TestSuite                                                                           \
	{                                                                                                                  \
	static auto& Name =                                                                                                \
		::inf::make_unit_test<inf::unit_tests_cls::TestSuite::Name>(INF_STRX(TestSuite), INF_STRX(Name));              \
	}                                                                                                                  \
	void inf::unit_tests_cls::TestSuite::Name::main()

#define INF_CUST_TEST_SUITE(Name)                                                                                      \
	namespace inf::test_suites_obj                                                                                     \
	{                                                                                                                  \
	static auto& Name = ::inf::make_test_suite(INF_STRX(Name));                                                        \
	}                                                                                                                  \
	class Name : public ::inf::unit_test

#define INF_CUST_UNIT_TEST(TestSuite, Name)                                                                            \
	namespace inf::unit_tests_cls::TestSuite                                                                           \
	{                                                                                                                  \
	class Name : public TestSuite                                                                                      \
	{                                                                                                                  \
	public:                                                                                                            \
		Name(std::string_view ts, std::string_view name)                                                               \
			: TestSuite{ ts, name }                                                                                    \
		{}                                                                                                             \
		Name(const Name&) = delete;                                                                                    \
		Name(Name&&) = delete;                                                                                         \
		Name& operator=(const Name&) = delete;                                                                         \
		Name& operator=(Name&&) = delete;                                                                              \
                                                                                                                       \
	protected:                                                                                                         \
		void main() override;                                                                                          \
	};                                                                                                                 \
	}                                                                                                                  \
	namespace inf::unit_tests_obj::TestSuite                                                                           \
	{                                                                                                                  \
	static auto& Name =                                                                                                \
		::inf::make_unit_test<inf::unit_tests_cls::TestSuite::Name>(INF_STRX(TestSuite), INF_STRX(Name));              \
	}                                                                                                                  \
	void inf::unit_tests_cls::TestSuite::Name::main()

#define INF_ASSERT(Assertion)                                                                                          \
	do                                                                                                                 \
	{                                                                                                                  \
		if (!(Assertion)) this->assert_fail("Assertion failed: " INF_STRX(Assertion));                                 \
	} while (false)

#define INF_EXPECT(Assertion)                                                                                          \
	do                                                                                                                 \
	{                                                                                                                  \
		if (!(Assertion)) this->expect_fail("Expectation failed: " INF_STRX(Assertion));                               \
	} while (false)

#define INF_RUN_TESTS()                                                                                                \
	int main()                                                                                                         \
	{                                                                                                                  \
		bool success = ::inf::test_suite::run_all();                                                                   \
		::inf::test_suite::display_all();                                                                              \
		return success ? EXIT_SUCCESS : EXIT_FAILURE;                                                                  \
	}

namespace inf
{

class test_suite;

class unit_test
{
public:
	unit_test() = delete;
	unit_test(unit_test const&) = delete;
	unit_test(unit_test&&) = default;

	unit_test& operator=(unit_test const&) = delete;
	unit_test& operator=(unit_test&&) = delete;

	virtual void before() {}
	virtual void main() {}
	virtual void after() {}

protected:
	unit_test(std::string_view suite_name, std::string_view name)
		: suite_name_{ suite_name }
		, name_{ name }
		, failures_{}
		, out_{}
		, err_{}
	{}

	void expect_fail(std::string_view what, source_location location = source_location::current())
	{
		failures_.emplace_back(location, what);
	}

	void assert_fail(std::string_view what, source_location location = source_location::current())
	{
		expect_fail(what, location);
		throw test_exception(what, location);
	}

private:
	friend test_suite;

	std::string_view name() const { return name_; }

	std::span<std::pair<source_location, std::string> const> failures() const
	{
		return { failures_.data(), failures_.size() };
	}

	bool run()
	{
		using namespace std::string_literals;
		int pid = io::getpid();
		try
		{
			this->before();
			if (pid != io::getpid()) std::abort();
		}
		catch (...)
		{
			expect_fail("test failed during before()");
			return false;
		}
		INF_AT_SCOPE_END({
			try
			{
				this->after();
				if (pid != io::getpid()) std::abort();
			}
			catch (...)
			{
				expect_fail("test failed during after()");
			}
		});
		try
		{
			this->main();
			if (pid != io::getpid()) std::abort();
		}
		catch (test_exception& e)
		{}
		catch (exception& e)
		{
			expect_fail("received inf::exception during test: "s + e.what(), e.location());
		}
		catch (std::exception& e)
		{
			expect_fail("received std::exception during test: "s + e.what());
		}
		catch (...)
		{
			expect_fail("received exception during test");
		}
		return !has_failed();
	}

	auto arun() { return std::async(std::launch::async, &unit_test::run, this); }

	void display()
	{
		std::string line;
		std::istringstream out_iss{ out_ };
		while (out_iss)
		{
			std::getline(out_iss, line);
			if (!line.empty() || out_iss)
				inf::cout << suite_name_ << "{}::" << name_ << "(): [out]: " << line << std::endl;
		}
		std::istringstream err_iss{ err_ };
		while (err_iss)
		{
			std::getline(err_iss, line);
			if (!line.empty() || err_iss)
				inf::cout << suite_name_ << "{}::" << name_ << "(): [err]: " << line << std::endl;
		}
		for (auto [location, what]: failures_)
			inf::cout << suite_name_ << "{}::" << name_ << "(): failure at " << location.file_name() << ':'
					  << location.line() << ':' << location.column() << ": " << what << std::endl;
		inf::cout << suite_name_ << "{}::" << name_ << "(): " << (has_failed() ? "failure" : "success") << std::endl;
	}

	bool has_failed() const { return !failures_.empty(); }

	std::string suite_name_;
	std::string name_;
	std::vector<std::pair<source_location, std::string>> failures_;
	std::string out_;
	std::string err_;
};

template <typename UnitTest>
inline unit_test& make_unit_test(std::string test_suite_name, std::string_view name,
								 source_location location = source_location::current());

class test_suite
{
public:
	test_suite(test_suite const&) = delete;
	test_suite(test_suite&&) = default;

	test_suite& operator=(test_suite const&) = delete;
	test_suite& operator=(test_suite&&) = delete;

	template <typename UnitTest>
	friend inline unit_test& make_unit_test(std::string test_suite_name, std::string_view name,
											source_location location);
	friend inline test_suite& make_test_suite(std::string_view name, source_location location);

	std::string_view name() const { return name_; }

	bool run()
	{
		std::vector<decltype(std::declval<unit_test>().arun())> promises;
		for (auto& [name, test]: tests_)
			promises.emplace_back(test->arun());
		bool success = true;
		for (auto& promise: promises)
			success &= promise.get();
		return success;
	}

	auto arun() { return std::async(std::launch::async, &test_suite::run, this); }

	void display()
	{
		for (auto& [name, test]: tests_)
			test->display();
		inf::cout << name_ << "{}: " << (has_failed() ? "failure" : "success") << std::endl;
	}

	bool has_failed() const
	{
		for (auto& [name, test]: tests_)
			if (test->has_failed()) return true;
		return false;
	}

	static bool run_all()
	{
		std::vector<decltype(std::declval<test_suite>().arun())> promises;
		for (auto& [name, suite]: test_suites_)
			promises.emplace_back(suite.arun());
		bool success = true;
		for (auto& promise: promises)
			success &= promise.get();
		return success;
	}

	static void display_all()
	{
		for (auto& [name, suite]: test_suites_)
			suite.display();
	}

private:
	test_suite(std::string_view name)
		: name_{ name }
		, tests_{}
	{}

	static std::unordered_map<std::string, test_suite> test_suites_;

	std::string name_;
	std::unordered_map<std::string, std::unique_ptr<unit_test>> tests_;
};

#if defined(INF_HEADER_ONLY) || defined(INF_STATIC_TESTS)
std::unordered_map<std::string, test_suite> test_suite::test_suites_{};
#endif

template <typename UnitTest>
inline unit_test& make_unit_test(std::string test_suite_name, std::string_view name, source_location location)
{
	auto test_suite_it = test_suite::test_suites_.find(test_suite_name);
	if (test_suite_it == test_suite::test_suites_.end()) throw exception("no inf::test_suite by that name", location);
	auto& test_suite = test_suite_it->second;
	auto [it, success] =
		test_suite.tests_.insert({ std::string{ name }, std::make_unique<UnitTest>(test_suite.name(), name) });
	if (!success) throw exception("two inf::unit_test with the same name", location);
	return *it->second;
}

inline test_suite& make_test_suite(std::string_view name, source_location location = source_location::current())
{
	auto [it, success] = test_suite::test_suites_.insert({ std::string{ name }, test_suite{ name } });
	if (!success) throw exception("two inf::test_suite with the same name", location);
	return it->second;
}

} // namespace inf
