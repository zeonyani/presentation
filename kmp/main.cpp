#include "kmp.hpp"

#include <chrono>
#include <cstring>
#include <functional>
#include <iomanip>
#include <iostream>
#include <random>
#include <string>
#include <vector>

// -------------------------------------------------------
// 캐시 플러시
// -------------------------------------------------------
static void flushCache() {
    constexpr std::size_t FLUSH_SIZE = 64ULL * 1024 * 1024;
    static char buf[FLUSH_SIZE];
    static bool inited = false;
    if (!inited) { std::memset(buf, 1, FLUSH_SIZE); inited = true; }
    volatile char sink = 0;
    for (std::size_t i = 0; i < FLUSH_SIZE; i += 64)
        sink += buf[i];
    (void)sink;
}

template <typename Fn>
long long measureNs(Fn&& fn) {
    flushCache();
    auto start = std::chrono::steady_clock::now();
    fn();
    auto end = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
}

long long medianNs(std::vector<long long> v) {
    std::sort(v.begin(), v.end());
    return v[v.size() / 2];
}

// -------------------------------------------------------
// strstr 래퍼
// -------------------------------------------------------
std::vector<int> strstrSearch(const std::string& text, const std::string& pattern) {
    std::vector<int> result;
    const char* t = text.c_str();
    const char* p = pattern.c_str();
    const char* pos = t;
    while ((pos = strstr(pos, p)) != nullptr) {
        result.push_back(static_cast<int>(pos - t));
        ++pos;
    }
    return result;
}

// -------------------------------------------------------
// std::string::find 래퍼
// -------------------------------------------------------
std::vector<int> findSearch(const std::string& text, const std::string& pattern) {
    std::vector<int> result;
    std::size_t pos = 0;
    while ((pos = text.find(pattern, pos)) != std::string::npos) {
        result.push_back(static_cast<int>(pos));
        ++pos;
    }
    return result;
}

// -------------------------------------------------------
// 벤치마크 헬퍼
// -------------------------------------------------------
struct BenchResult {
    std::string name;
    long long   medNs;
    std::size_t matchCount;
};

BenchResult bench(const std::string& name, int repeat,
                  std::function<std::vector<int>()> fn) {
    std::vector<long long> samples;
    std::size_t cnt = 0;
    for (int i = 0; i < repeat; ++i) {
        std::vector<int> r;
        long long ns = measureNs([&]{ r = fn(); });
        samples.push_back(ns);
        cnt = r.size();
    }
    return { name, medianNs(samples), cnt };
}

void printResults(const std::vector<BenchResult>& res) {
    long long kmpMed = 0;
    for (auto& r : res)
        if (r.name.find("KMP") != std::string::npos) kmpMed = r.medNs;

    for (auto& r : res) {
        std::cout << std::fixed << std::setprecision(3);
        std::cout << "  " << std::left << std::setw(8) << r.name
                  << " : " << std::right << std::setw(8) << r.medNs/1e6 << " ms"
                  << "  (매칭 " << r.matchCount << "개)"
                  << "  x" << std::setprecision(2) << (double)r.medNs/kmpMed
                  << "\n";
    }
}

// -------------------------------------------------------
// 랜덤 문자열
// -------------------------------------------------------
std::string makeRandom(std::size_t len, int alpha, unsigned seed) {
    std::mt19937 rng(seed);
    std::uniform_int_distribution<int> d(0, alpha - 1);
    std::string s(len, ' ');
    for (auto& c : s) c = static_cast<char>('a' + d(rng));
    return s;
}

int main() {
    constexpr std::size_t TEXT_LEN    = 1'000'000;
    constexpr std::size_t PATTERN_LEN = 1'000;
    constexpr int         REPEAT      = 5;

    // -------------------------------------------------------
    // 정합성 확인
    // -------------------------------------------------------
    {
        std::string t = "ABABABABCABABABABC", p = "ABABC";
        auto r1 = kmpSearch(t, p), r2 = naiveSearch(t, p);
        auto r3 = strstrSearch(t, p), r4 = findSearch(t, p);
        std::cout << "[정합성 확인]\n";
        auto print = [](const std::string& name, const std::vector<int>& v) {
            std::cout << "  " << std::left << std::setw(8) << name << ": ";
            for (int i : v) std::cout << i << ' ';
            std::cout << '\n';
        };
        print("KMP",    r1); print("Naive",  r2);
        print("strstr", r3); print("find",   r4);
        std::cout << "  모두 일치: "
                  << ((r1==r2 && r2==r3 && r3==r4) ? "OK" : "MISMATCH!") << "\n\n";
    }

    // -------------------------------------------------------
    // 케이스 1: 최악 (aaa...a + 끝 b)
    // -------------------------------------------------------
    std::cout << "=== 케이스 1: 최악 (텍스트 전부 'a', 패턴 끝만 'b') ===\n";
    {
        std::string text(TEXT_LEN, 'a');
        std::string pat(PATTERN_LEN - 1, 'a'); pat += 'b';
        std::vector<BenchResult> res;
        res.push_back(bench("KMP",    REPEAT, [&]{ return kmpSearch   (text, pat); }));
        res.push_back(bench("Naive",  REPEAT, [&]{ return naiveSearch  (text, pat); }));
        res.push_back(bench("strstr", REPEAT, [&]{ return strstrSearch (text, pat); }));
        res.push_back(bench("find",   REPEAT, [&]{ return findSearch   (text, pat); }));
        printResults(res);
    }

    // -------------------------------------------------------
    // 케이스 2: 평균 (랜덤, 알파벳 4종)
    // -------------------------------------------------------
    std::cout << "\n=== 케이스 2: 평균 (랜덤 텍스트, 알파벳 4종) ===\n";
    {
        std::string text = makeRandom(TEXT_LEN,    4, 42);
        std::string pat  = makeRandom(PATTERN_LEN, 4, 99);
        std::vector<BenchResult> res;
        res.push_back(bench("KMP",    REPEAT, [&]{ return kmpSearch   (text, pat); }));
        res.push_back(bench("Naive",  REPEAT, [&]{ return naiveSearch  (text, pat); }));
        res.push_back(bench("strstr", REPEAT, [&]{ return strstrSearch (text, pat); }));
        res.push_back(bench("find",   REPEAT, [&]{ return findSearch   (text, pat); }));
        printResults(res);
    }

    // -------------------------------------------------------
    // 케이스 3: 평균 (랜덤, 알파벳 26종) — 불일치 더 빠름
    // -------------------------------------------------------
    std::cout << "\n=== 케이스 3: 평균 (랜덤 텍스트, 알파벳 26종) ===\n";
    {
        std::string text = makeRandom(TEXT_LEN,    26, 42);
        std::string pat  = makeRandom(PATTERN_LEN, 26, 99);
        std::vector<BenchResult> res;
        res.push_back(bench("KMP",    REPEAT, [&]{ return kmpSearch   (text, pat); }));
        res.push_back(bench("Naive",  REPEAT, [&]{ return naiveSearch  (text, pat); }));
        res.push_back(bench("strstr", REPEAT, [&]{ return strstrSearch (text, pat); }));
        res.push_back(bench("find",   REPEAT, [&]{ return findSearch   (text, pat); }));
        printResults(res);
    }

    return 0;
}