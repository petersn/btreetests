// Testing

#include <cmath>
#include <cassert>
#include <cstdint>
#include <vector>
#include <unordered_map>
#include <map>
#include <random>
#include <iostream>
#include <chrono>

extern "C" {
	#include "interface.h"
}

//#define DEBUG_EFFICIENCY

constexpr int NUM_RUNS = 1;

std::mt19937 rng(123456789);

constexpr double EULER_MASCHERONI = 0.5772156649015328606065121;

int approximate_zipf_sample(int max) {
	// Recall that the distribution we're trying to sample is:
	//   Support: {1, 2, ... N}
	//   CDF: H(k) / H(N)
	// Therefore, our inverse CDF is approximately:
	//   k = H⁻¹(p · H(n))
	// Note that H(n) ≈ log(n) + γ, and H⁻¹(x) ≈ exp(x - γ)
	double x = std::uniform_real_distribution<double>(0, 1)(rng);
	double scale = std::log(max) + EULER_MASCHERONI;
	double result = std::exp(x * scale - EULER_MASCHERONI);
	return std::round(result);
}

struct Timer {
	std::chrono::time_point<std::chrono::high_resolution_clock> timer_start;

	Timer() {
		timer_start = std::chrono::high_resolution_clock::now();
	}

	double stop() {
		auto timer_stop = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double> elapsed = timer_stop - timer_start;
		return elapsed.count();
	}
};

double sequential_insert(int reps, int count, bool reverse) {
	std::vector<char> m_buffer(map_size);
	void* m = static_cast<void*>(&m_buffer[0]);
	Timer t;
	for (int r = 0; r < reps; r++) {
		map_alloc(m);
		for (int i = 0; i < count; i++)
			map_assign(m, reverse ? -i : i, i);
		Value v;
		for (int i = 0; i < count; i++)
			map_lookup(m, reverse ? -i : i, &v);
		map_free(m);
	}
	double elapsed = t.stop();
#ifdef DEBUG_EFFICIENCY
	printf("%8.2fms sequential reps=%i keys=%i reverse=%i\n", 1e3 * elapsed, reps, count, reverse);
#endif
	return elapsed;
}

enum Distribution {
	UNIFORM,
	ZIPF,
};

enum ActionKind {
	ASSIGN,
	LOOKUP,
	DELETE,
	RANGE_LOOKUP,
};

struct Action {
	ActionKind kind;
	Key key;
	Value value;
	Key key_high;
};

void populate_actions(
	std::vector<Action>& actions,
	int action_count,
	int distinct_keys,
	Distribution dist,
	bool include_range_lookups
) {
	std::vector<Key> key_index_to_key;
	std::uniform_int_distribution<Key> key_distribution;
	for (int i = 0; i < distinct_keys; i++)
		key_index_to_key.push_back(key_distribution(rng));

	std::uniform_int_distribution<Value> value_distribution;
	for (int r = 0; r < action_count; r++) {
		auto max_action = include_range_lookups ? RANGE_LOOKUP : DELETE;
		ActionKind kind = static_cast<ActionKind>(
			std::uniform_int_distribution<int>(ASSIGN, max_action)(rng)
		);
		int key_index;
		switch (dist) {
			case UNIFORM:
				key_index = std::uniform_int_distribution<int>(0, key_index_to_key.size() - 1)(rng);
				break;
			case ZIPF:
				key_index = approximate_zipf_sample(distinct_keys) - 1;
				break;
			default: assert(0);
		}
		Key key = key_index_to_key.at(key_index);
		Value value = value_distribution(rng);
		Key key_high = 0;

		// One endpoint is uniform, and the other is a distributed key from the container.
		if (kind == RANGE_LOOKUP) {
			key_high = key_distribution(rng);
			if (key_high < key)
				std::swap(key, key_high);
		}

		actions.push_back({kind, key, value, key_high});
	}
}

double random_usage(int action_count, int distinct_keys, Distribution dist, bool include_range_lookups, int range_query_size) {
	std::vector<char> m_buffer(map_size);
	void* m = static_cast<void*>(&m_buffer[0]);

	// To reduce the effect of generating random data during the test pregenerate all the actions.
	std::vector<Action> actions;
	populate_actions(actions, action_count, distinct_keys, dist, include_range_lookups);
	std::vector<KVPair> range_results_buffer(distinct_keys);

	Timer t;
	map_alloc(m);
	for (auto& action : actions) {
		switch (action.kind) {
			case ASSIGN:
				map_assign(m, action.key, action.value);
				break;
			case LOOKUP:
				map_lookup(m, action.key, &action.value);
				break;
			case DELETE:
				map_delete(m, action.key, &action.value);
				break;
			case RANGE_LOOKUP:
				map_lookup_range(m, action.key, action.key_high, range_query_size, &range_results_buffer[0]);
				break;
			default: assert(0);
		}
	}
	map_free(m);
	double elapsed = t.stop();
#ifdef DEBUG_EFFICIENCY
	printf("%8.2fms random actions=%i keys=%i dist=%i range=%i\n", 1e3 * elapsed, action_count, distinct_keys, (int)dist, include_range_lookups ? range_query_size : -1);
#endif
	return elapsed;
}

template <typename T>
void assert_equal(const char* message, T x, T y) {
	if (x == y)
		return;
	std::cerr << "Error: " << message << " -- got: " << x << " expected: " << y << std::endl;
	assert(0);
}

void check_correctness() {
	std::vector<char> m_buffer(map_size);
	void* m = static_cast<void*>(&m_buffer[0]);

	const int action_count = 10000;
	const int distinct_keys = 1000;
	const int max_range_query_size = distinct_keys + 10;

	std::vector<Action> actions;
	populate_actions(actions, action_count, distinct_keys, ZIPF, true);
	std::vector<KVPair> range_results_buffer(max_range_query_size);

	// When calling range queries we want a sentinel to check whether a write happened.
	std::uniform_int_distribution<Key> key_distribution;
	std::uniform_int_distribution<Value> value_distribution;
	Value key_sentinel = key_distribution(rng);
	Value value_sentinel = value_distribution(rng);

	std::map<Key, Value> reference_map;
	map_alloc(m);
	for (auto& action : actions) {
		// 10% of the time opt not to get the value, to check that behavior.
		bool do_get_value = std::uniform_real_distribution<float>(0, 1)(rng) < 0.9;

		switch (action.kind) {
			case ASSIGN: {
				map_assign(m, action.key, action.value);
				reference_map[action.key] = action.value;
				break;
			}
			case LOOKUP:
			case DELETE: {
				action.value = value_sentinel;
				size_t hit_count = (action.kind == DELETE ? map_delete : map_lookup)(
					m, action.key, do_get_value ? &action.value : nullptr
				);
				assert_equal<size_t>("bad lookup count", hit_count, reference_map.count(action.key));
				if (hit_count and do_get_value)
					assert_equal<Value>("bad lookup value", action.value, reference_map[action.key]);
				else
					assert_equal<Value>("sentinel overwritten", action.value, value_sentinel);
				if (action.kind == DELETE)
					reference_map.erase(action.key);
				break;
			}
			case RANGE_LOOKUP: {
				// Fill the results buffer with a sentinel.
				std::fill(
					range_results_buffer.begin(),
					range_results_buffer.end(),
					KVPair{key_sentinel, value_sentinel}
				);
				size_t range_query_size = std::uniform_int_distribution<size_t>(0, max_range_query_size)(rng);
				size_t hit_count = map_lookup_range(
					m, action.key, action.key_high, range_query_size, do_get_value ? &range_results_buffer[0] : nullptr
				);
				// Reference query.
				auto it = reference_map.lower_bound(action.key);
				size_t reference_hit_count = 0;
				while (it != reference_map.end() and (*it).first <= action.key_high and range_query_size--) {
					auto p = *it++;
					if (do_get_value) {
						assert_equal<Key>
							("bad range result key",
							range_results_buffer[reference_hit_count].key,
							p.first
						);
						assert_equal<Key>
							("bad range result value",
							range_results_buffer[reference_hit_count].value,
							p.second
						);
					}
					reference_hit_count++;
				}
				assert_equal<size_t>("bad range result length", hit_count, reference_hit_count);
				break;
			}
			default: assert(0);
		}
	}
	map_free(m);
}

double weighted_points(const std::vector<std::pair<double, double>> scores, double target_weight) {
	// Compute a weighted geometric mean time.
	double total_benchmark_time = 0;
	double log_accum = 0;
	double total_weight = 0;
	for (auto p : scores) {
		log_accum += p.first * std::log(p.second);
		total_weight += p.first;
		total_benchmark_time += p.second;
		if (p.second < 1e-5) {
			std::cerr << "WARNING: Unexpectedly low time on a subtask!" << std::endl;
			std::cerr << "This might throw off the geometric mean." << std::endl;
		}
#ifdef DEBUG_EFFICIENCY
		std::cout << " ms: " << 1e3 * p.second << std::endl;
#endif
	}
	assert(fabs(total_weight - target_weight) < 1e-6);
#ifdef DEBUG_EFFICIENCY
//	std::cout << "Benchmark efficiency: " << total_benchmark_time / efficiency_timer.stop() << std::endl;
#endif
	double geometric_mean_time = std::exp(log_accum / total_weight);

	return 1.0 / geometric_mean_time;
}

double run_benchmark() {
//	Timer efficiency_timer;

	// Section 1: Linear insertions followed by linear lookup. (Total weight: 0.5)
	std::vector<std::pair<double, double>> section1_scores;
	for (bool reverse : {false, true}) {
		for (auto p : {
			std::pair<int, int>
			{ 50000, 10},
			{ 5000, 100},
			{ 500, 1000},
			{ 50, 10000},
			{ 5, 100000},
			{1, 1000000},
		}) {
			double t = sequential_insert(p.first, p.second, reverse);
			// There are twelve benchmarks in this section, so give them each 1/24 weight.
			section1_scores.emplace_back(1 / 24.0, t);
		}
	}

	// Section 2: Random insertions, deletions, and lookups. (Total weight: 2)
	std::vector<std::pair<double, double>> section2_scores;
	std::vector<std::pair<double, double>> section2_scores_uniform;
	std::vector<std::pair<double, double>> section2_scores_zipf;
	for (auto dist : {UNIFORM, ZIPF}) {
		for (auto distinct_keys : {
			100,
			1000,
			10000,
			100000,
			1000000,
		}) {
			section2_scores.emplace_back(1 / 5.0, random_usage(1000000, distinct_keys, dist, false, 0));
			(dist == UNIFORM ? section2_scores_uniform : section2_scores_zipf).push_back(section2_scores.back());
		}
	}

	// Section 3: Random range lookups. (Total weight: 2)
	std::vector<std::pair<double, double>> section3_scores;
	std::vector<std::pair<double, double>> section3_scores_uniform;
	std::vector<std::pair<double, double>> section3_scores_zipf;
	for (auto dist : {UNIFORM, ZIPF}) {
		for (auto distinct_keys : {10000, 1000000}) {
			for (auto range_query_size : {4, 64, 1024}) {
				int action_count = 200000;
				if (range_query_size == 1024)
					action_count /= 4;
				section3_scores.emplace_back(
					1 / 6.0,
					random_usage(action_count, distinct_keys, dist, true, range_query_size)
				);
				(dist == UNIFORM ? section3_scores_uniform : section3_scores_zipf).push_back(section3_scores.back());
			}
		}
	}

	printf("Section 1: Linear insertion points:   %.2f\n", weighted_points(section1_scores, 0.5)         / 1.2);
	printf("Section 2: Random usage points:       %.2f\n", weighted_points(section2_scores, 2.0)         / 0.7);
	printf("    Uniform random usage points:      %.2f\n", weighted_points(section2_scores_uniform, 1.0) / 0.7);
	printf("    Zipf random usage points:         %.2f\n", weighted_points(section2_scores_zipf, 1.0)    / 0.7);
	printf("Section 3: Random range query points: %.2f\n", weighted_points(section3_scores, 2.0)         / 1.6);
	printf("    Uniform random usage points:      %.2f\n", weighted_points(section3_scores_uniform, 1.0) / 1.6);
	printf("    Zipf random usage points:         %.2f\n", weighted_points(section3_scores_zipf, 1.0)    / 1.6);

	std::vector<std::pair<double, double>> scores;
	scores.insert(scores.end(), section1_scores.begin(), section1_scores.end());
	scores.insert(scores.end(), section2_scores.begin(), section2_scores.end());
	scores.insert(scores.end(), section3_scores.begin(), section3_scores.end());
	return weighted_points(scores, 4.5);
}

int main() {
	std::cout << "Checking correctness... " << std::flush;
	for (int i = 0; i < 100; i++)
		check_correctness();
	std::cout << "pass." << std::endl;

	std::cout << "Map size in bytes: " << map_size << std::endl;
	double best = 0;
	for (int run_index = 0; run_index < NUM_RUNS; run_index++) {
		double points = run_benchmark();
		best = std::max(best, points);
		if (NUM_RUNS != 1)
			printf("Run %i points: %.2f\n", (run_index + 1), points);
	}
	std::cout << std::endl;
	printf("Benchmark points: %.2f\n", best);
}

