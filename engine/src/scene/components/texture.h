#pragma once

namespace engine
{
    namespace sg
    {
        class Image;
        class Sampler;

        class Texture
        {
        public:
            Texture(const std::string &name);
            ~Texture();

            Texture(Texture &&other) = default;

            void SetImage(Image &image) { m_Image = &image; }

            Image *GetImage() { return m_Image; }

            void SetSampler(Sampler &sampler) { m_Sampler = &sampler; }

            Sampler *GetSampler()
            {
                ENG_ASSERT(m_Sampler, "Texture has no sampler");
                return m_Sampler;
            }

        private:
            //TODO
            std::string m_Name;
            Image *m_Image{nullptr};
            Sampler *m_Sampler{nullptr};
        };
    }
}
