#pragma once
#include <string>
#include <vector>

// 실패 함수(failure function) 계산
// f[i] = P[0..i]에서 접두사=접미사인 가장 긴 문자열의 길이
std::vector<int> buildFailure(const std::string& pattern);

// KMP 탐색: 텍스트에서 패턴이 등장하는 모든 시작 인덱스 반환
std::vector<int> kmpSearch(const std::string& text, const std::string& pattern);

// Naive 탐색 (비교용): O(N*M)
std::vector<int> naiveSearch(const std::string& text, const std::string& pattern);