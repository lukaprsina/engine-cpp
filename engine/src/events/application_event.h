#pragma once

#include "events/event.h"

namespace engine
{
    class Window;

    class WindowCloseEvent : public Event
    {
    public:
        WindowCloseEvent(Window *window)
            : Event(window) {}

        std::string ToString() const override
        {
            std::stringstream ss;
            ss << "WindowCloseEvent";
            return ss.str();
        }

        EVENT_CLASS_TYPE(WindowClose)
        EVENT_CLASS_CATEGORY(EventCategoryApplication)
    };

    class WindowResizeEvent : public Event
    {
    public:
        WindowResizeEvent(Window *window, unsigned int width, unsigned int height)
            : Event(window), m_Width(width), m_Height(height) {}

        inline unsigned int GetWidth() const { return m_Width; }
        inline unsigned int GetHeight() const { return m_Height; }

        std::string ToString() const override
        {
            std::stringstream ss;
            ss << "WindowResizeEvent: " << m_Width << ", " << m_Height;
            return ss.str();
        }

        EVENT_CLASS_TYPE(WindowResize)
        EVENT_CLASS_CATEGORY(EventCategoryApplication)
    private:
        unsigned int m_Width, m_Height;
    };

    class WindowFocusedEvent : public Event
    {
    public:
        WindowFocusedEvent(Window *window, int Focused)
            : Event(window), m_Focused((bool)Focused)
        {
        }

        std::string ToString() const override
        {
            std::stringstream ss;
            const char *description = (m_Focused) ? "Focus gained." : "Focus lost.";

            ss << "WindowFocusedEvent: " << description;
            return ss.str();
        }

        EVENT_CLASS_TYPE(WindowFocus)
        EVENT_CLASS_CATEGORY(EventCategoryApplication)

    private:
        bool m_Focused;
    };

    class WindowMovedEvent : public Event
    {
    public:
        WindowMovedEvent(Window *window, int XPositon, int YPosition)
            : Event(window), m_XPosition(XPositon), m_YPosition(YPosition) {}

        inline int GetXPosition() const { return m_XPosition; }
        inline int GetYPosition() const { return m_YPosition; }

        std::string ToString() const override
        {
            std::stringstream ss;
            ss << "WindowMovedEvent: " << m_XPosition << ", " << m_YPosition;
            return ss.str();
        }

        EVENT_CLASS_TYPE(WindowMoved)
        EVENT_CLASS_CATEGORY(EventCategoryApplication)
    private:
        int m_XPosition, m_YPosition;
    };
}