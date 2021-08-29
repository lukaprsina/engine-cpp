#pragma once

namespace engine
{
    namespace sg
    {
        class Script
        {
        public:
            Script();
            virtual ~Script();

            virtual void Update(float delta_time) = 0;
            virtual void Resize(uint32_t width, uint32_t height){};
        };
    }
}
