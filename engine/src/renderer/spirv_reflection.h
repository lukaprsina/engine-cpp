/* Copyright (c) 2019, Arm Limited and Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 the "License";
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

ENG_DISABLE_WARNINGS()
#include <spirv_glsl.hpp>
ENG_ENABLE_WARNINGS()

#include "renderer/shader.h"

namespace engine
{
    class SPIRVReflection
    {
    public:
        bool ReflectShaderResources(VkShaderStageFlagBits stage,
                                    const std::vector<uint32_t> &spirv,
                                    std::vector<ShaderResource> &resources,
                                    const ShaderVariant &variant);

    private:
        void ParseShaderResources(const spirv_cross::Compiler &compiler,
                                  VkShaderStageFlagBits stage,
                                  std::vector<ShaderResource> &resources,
                                  const ShaderVariant &variant);

        void ParsePushConstants(const spirv_cross::Compiler &compiler,
                                VkShaderStageFlagBits stage,
                                std::vector<ShaderResource> &resources,
                                const ShaderVariant &variant);

        void ParseSpecializationConstants(const spirv_cross::Compiler &compiler,
                                          VkShaderStageFlagBits stage,
                                          std::vector<ShaderResource> &resources,
                                          const ShaderVariant &variant);
    };
}
