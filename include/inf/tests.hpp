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
// inf
#include <inf/exceptions.hpp>
#include <inf/pipe.hpp>
#include <inf/source_location.hpp>
#include <inf/stdio_stream.hpp>
#include <inf/utils.hpp>

#define INF_TEST_SUITE(Name)                                                                                           \
	namespace inf::test_suites_obj                                                                                     \
	{                                                                                                                  \
	static auto& Name = ::inf::make_test_suite(INF_STRX(Name));                                                        \
	}

#define INF_UNIT_TEST(TestSuite, Name, ...)                                                                            \
	namespace inf::unit_tests_obj::TestSuite                                                                           \
	{                                                                                                                  \
	static auto& Name = ::inf::make_unit_test(INF_STRX(TestSuite), INF_STRX(Name),                                     \
											  []([[maybe_unused]] unit_test& context) { __VA_ARGS__; });               \
	}

#define INF_ASSERT(Assertion)                                                                                          \
	do                                                                                                                 \
	{                                                                                                                  \
		if (!(Assertion)) context.assert_fail("Assertion failed: " INF_STRX(Assertion));                               \
	} while (false)

#define INF_EXPECT(Assertion)                                                                                          \
	do                                                                                                                 \
	{                                                                                                                  \
		if (!(Assertion)) context.expect_fail("Expecation failed: " INF_STRX(Assertion));                              \
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
	using func_type = std::function<void(unit_test&)>;

	unit_test(unit_test const&) = delete;
	unit_test(unit_test&&) = default;

	unit_test& operator=(unit_test const&) = delete;
	unit_test& operator=(unit_test&&) = delete;

	friend inline unit_test& make_unit_test(std::string test_suite_name, std::string_view name, func_type func,
											source_location location);

	std::string_view name() const { return name_; }

	std::span<std::pair<source_location, std::string> const> failures() const
	{
		return { failures_.data(), failures_.size() };
	}

	void expect_fail(std::string_view what, source_location location = source_location::current())
	{
		failures_.emplace_back(location, what);
	}

	void assert_fail(std::string_view what, source_location location = source_location::current())
	{
		expect_fail(what, location);
		throw test_exception(what, location);
	}

	bool run()
	{
		using namespace std::string_literals;
		/*auto out_pipe = inf::make_pipe();
		auto err_pipe = inf::make_pipe();
		auto out_bck = out_pipe.write.safe_dup(inf::cout);
		auto err_bck = err_pipe.write.safe_dup(inf::cerr);
		INF_AT_SCOPE_END({
			out_bck.dup_back(inf::cout);
			err_bck.dup_back(inf::cerr);
			out_ = std::string{ std::istreambuf_iterator<char>(out_pipe.read), {} };
			err_ = std::string{ std::istreambuf_iterator<char>(err_pipe.read), {} };
		});*/
		try
		{
			func_(*this);
		}
		catch (test_exception& e)
		{
			return false;
		}
		catch (exception& e)
		{
			expect_fail("received inf::exception during test: "s + e.what(), e.location());
			return false;
		}
		catch (std::exception& e)
		{
			expect_fail("received std::exception during test: "s + e.what());
			return false;
		}
		catch (...)
		{
			expect_fail("received exception during test");
			return false;
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

private:
	unit_test(std::string_view suite_name, std::string_view name, func_type func)
		: suite_name_{ suite_name }
		, name_{ name }
		, func_{ func }
		, failures_{}
	{}

	std::string suite_name_;
	std::string name_;
	func_type func_;
	std::vector<std::pair<source_location, std::string>> failures_;
	std::string out_;
	std::string err_;
};

class test_suite
{
public:
	test_suite(test_suite const&) = delete;
	test_suite(test_suite&&) = default;

	test_suite& operator=(test_suite const&) = delete;
	test_suite& operator=(test_suite&&) = delete;

	friend inline test_suite& make_test_suite(std::string_view name, source_location location);
	friend inline unit_test& make_unit_test(std::string test_suite_name, std::string_view name,
											unit_test::func_type func, source_location location);

	std::string_view name() const { return name_; }

	bool run()
	{
		std::vector<decltype(std::declval<unit_test>().arun())> promises;
		for (auto& [name, test]: tests_)
			promises.emplace_back(test.arun());
		bool success = true;
		for (auto& promise: promises)
			success &= promise.get();
		return success;
	}

	auto arun() { return std::async(std::launch::async, &test_suite::run, this); }

	void display()
	{
		for (auto& [name, test]: tests_)
			test.display();
		inf::cout << name_ << "{}: " << (has_failed() ? "failure" : "success") << std::endl;
	}

	bool has_failed() const
	{
		for (auto& [name, test]: tests_)
			if (test.has_failed()) return true;
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
	std::unordered_map<std::string, unit_test> tests_;
};

std::unordered_map<std::string, test_suite> test_suite::test_suites_{};

inline unit_test& make_unit_test(std::string test_suite_name, std::string_view name, unit_test::func_type func,
								 source_location location = source_location::current())
{
	auto test_suite_it = test_suite::test_suites_.find(test_suite_name);
	if (test_suite_it == test_suite::test_suites_.end()) throw exception("no inf::test_suite by that name", location);
	auto& test_suite = test_suite_it->second;
	auto [it, success] = test_suite.tests_.insert({
		std::string{ name },
		 unit_test{ test_suite.name(), name, func }
	});
	if (!success) throw exception("two inf::unit_test with the same name", location);
	return it->second;
}

inline test_suite& make_test_suite(std::string_view name, source_location location = source_location::current())
{
	auto [it, success] = test_suite::test_suites_.insert({ std::string{ name }, test_suite{ name } });
	if (!success) throw exception("two inf::test_suite with the same name", location);
	return it->second;
}

} // namespace inf
