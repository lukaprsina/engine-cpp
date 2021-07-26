#pragma once

#include <iostream>
#include <vector>
#include <memory>
#include <filesystem>
#include <map>
#include <chrono>
#include <thread>
#include <assert.h>
#include <array>

#include "core/log.h"
#include "common/base.h"
#include "common/error.h"
#include "common/strings.h"
#include "common/vulkan.h"

namespace fs = std::filesystem;