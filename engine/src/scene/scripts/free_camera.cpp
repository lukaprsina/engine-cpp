#include "scene/scripts/free_camera.h"

#include "scene/components/perspective_camera.h"
#include "scene/components/transform.h"
#include "core/application.h"
#include "scene/scene.h"
#include "core/layer.h"
#include "window/input.h"
#include "window/window.h"

#include "common/glm.h"
ENG_DISABLE_WARNINGS()
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/string_cast.hpp>
ENG_ENABLE_WARNINGS()

namespace engine
{
    namespace sg
    {
        FreeCamera::FreeCamera(Application &application, std::string &layer_name)
            : m_App(application), m_LayerName(layer_name)
        {
        }

        FreeCamera::~FreeCamera()
        {
        }

        FreeCamera::FreeCamera(FreeCamera &&other)
            : m_App(other.m_App), m_LayerName(other.m_LayerName)
        {
        }

        FreeCamera &FreeCamera::operator=(FreeCamera &&other)
        {
            return *this;
        }

        void FreeCamera::Update(float delta_time)
        {
            glm::vec3 delta_translation(0.0f, 0.0f, 0.0f);
            glm::vec3 delta_rotation(0.0f, 0.0f, 0.0f);
            float mul_translation = m_SpeedMultiplier;

            Layer *layer = m_App.GetLayerStack().GetLayers().at(m_LayerName).get();
            Window *window = layer->GetWindow();

            if (window->GetInput().IsKeyPressed(Key::W))
                delta_translation.z -= 50.0f;

            if (window->GetInput().IsKeyPressed(Key::S))
                delta_translation.z += 50.0f;

            if (window->GetInput().IsKeyPressed(Key::A))
                delta_translation.x -= 50.0f;

            if (window->GetInput().IsKeyPressed(Key::D))
                delta_translation.x += 50.0f;

            if (window->GetInput().IsKeyPressed(Key::Up))
                delta_rotation.x -= 2.0f;

            if (window->GetInput().IsKeyPressed(Key::Down))
                delta_rotation.x += 2.0f;

            if (window->GetInput().IsKeyPressed(Key::Left))
                delta_rotation.y += 2.0f;

            if (window->GetInput().IsKeyPressed(Key::Right))
                delta_rotation.y -= 2.0f;

            if (window->GetInput().IsKeyPressed(Key::LeftShift))
                mul_translation /= 4;

            if (window->GetInput().IsKeyPressed(Key::LeftControl))
                mul_translation *= 4;

            delta_translation *= mul_translation * delta_time;
            delta_rotation *= delta_time;

            if (delta_rotation != glm::vec3(0.0f, 0.0f, 0.0f) || delta_translation != glm::vec3(0.0f, 0.0f, 0.0f))
            {
                auto &transform = layer->GetCamera()->GetComponent<sg::Transform>();
                glm::quat qx = glm::angleAxis(delta_rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
                glm::quat qy = glm::angleAxis(delta_rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));

                glm::quat orientation = glm::normalize(qy * transform.GetRotation() * qx);

                transform.SetTranslation(transform.GetTranslation() + delta_translation * glm::conjugate(orientation));
                transform.SetRotation(orientation);
            }

            m_MouseMoveDelta = {};
        }

        void FreeCamera::Resize(sg::PerspectiveCamera &camera, uint32_t width, uint32_t height)
        {
            camera.m_AspectRatio = static_cast<float>(width) / height;
        }
    }
}
