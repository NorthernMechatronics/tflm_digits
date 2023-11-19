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

#include "model_settings.h"

// These are the category labels mentioned within the dataset. You may customize
// them based on the trained model.
#if defined(MODEL_OPT)
const char kCategoryLabels[kCategoryCount] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
#else
const char kCategoryLabels[kCategoryCount] = {'1', '2', '3', '4', '5', '6', '7', '8', '9', '0'};
#endif