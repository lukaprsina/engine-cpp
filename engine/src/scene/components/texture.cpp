#include "scene/components/texture.h"

#include "scene/components/image.h"
#include "scene/components/sampler.h"

namespace engine
{
    namespace sg
    {
        Texture::Texture(const std::string &name)
            : m_Name(name)
        {
        }

        Texture::~Texture()
        {
        }
    }
}
