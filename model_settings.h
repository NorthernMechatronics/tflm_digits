/* Copyright 2019 The TensorFlow Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

#ifndef TENSORFLOW_LITE_MODEL_SETTINGS_H_
#define TENSORFLOW_LITE_MODEL_SETTINGS_H_

constexpr int kNumCols = 32;
constexpr int kNumRows = 32;
constexpr int kNumChannels = 3;

constexpr int kMaxImageSize = kNumCols * kNumRows * kNumChannels;

constexpr int kCategoryCount = 10;
constexpr int kOneIndex = 0;
constexpr int kTwoIndex = 1;
constexpr int kThreeIndex = 2;
constexpr int kFourIndex = 3;
constexpr int kFiveIndex = 4;
constexpr int kSixIndex = 5;
constexpr int kSevenIndex = 6;
constexpr int kEightIndex = 7;
constexpr int kNineIndex = 8;
constexpr int kZeroIndex = 9;
extern const char* kCategoryLabels[kCategoryCount];

#endif // TENSORFLOW_LITE_MODEL_SETTINGS_H_