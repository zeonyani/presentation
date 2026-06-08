#include "kmp.hpp"

// -------------------------------------------------------
// 실패 함수 구축  O(M)
// -------------------------------------------------------
std::vector<int> buildFailure(const std::string& pattern) {
    int m = static_cast<int>(pattern.size());
    std::vector<int> f(m, 0);

    // j: 현재까지 맞은 접두사 길이
    int j = 0;
    for (int i = 1; i < m; ++i) {
        // 불일치: 실패 함수를 따라 후퇴
        while (j > 0 && pattern[i] != pattern[j]) {
            j = f[j - 1];
        }
        // 일치: 접두사 길이 1 증가
        if (pattern[i] == pattern[j]) {
            ++j;
        }
        f[i] = j;
    }
    return f;
}

// -------------------------------------------------------
// KMP 탐색  O(N + M)
// -------------------------------------------------------
std::vector<int> kmpSearch(const std::string& text, const std::string& pattern) {
    std::vector<int> result;
    if (pattern.empty()) return result;

    int n = static_cast<int>(text.size());
    int m = static_cast<int>(pattern.size());
    std::vector<int> f = buildFailure(pattern);

    int j = 0;  // 패턴에서 현재 비교 위치
    for (int i = 0; i < n; ++i) {
        // 불일치: 실패 함수로 점프 (텍스트 포인터 i는 유지)
        while (j > 0 && text[i] != pattern[j]) {
            j = f[j - 1];
        }
        // 일치
        if (text[i] == pattern[j]) {
            ++j;
        }
        // 패턴 전체 매칭 완료
        if (j == m) {
            result.push_back(i - m + 1);   // 매칭 시작 인덱스
            j = f[j - 1];                   // 이어서 탐색 (중복 매칭 허용)
        }
    }
    return result;
}

// -------------------------------------------------------
// Naive 탐색  O(N * M)
// -------------------------------------------------------
std::vector<int> naiveSearch(const std::string& text, const std::string& pattern) {
    std::vector<int> result;
    if (pattern.empty()) return result;

    int n = static_cast<int>(text.size());
    int m = static_cast<int>(pattern.size());

    for (int i = 0; i <= n - m; ++i) {
        int j = 0;
        while (j < m && text[i + j] == pattern[j]) {
            ++j;
        }
        if (j == m) {
            result.push_back(i);
        }
    }
    return result;
}