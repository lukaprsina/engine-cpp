#pragma once

namespace engine
{
    class Window;

    enum class EventType
    {
        None = 0,
        WindowClose,
        WindowResize,
        WindowFocus,
        WindowMoved,
        KeyPressed,
        KeyReleased,
        MouseButtonPressed,
        MouseButtonReleased,
        MouseMoved,
        MouseScrolled
    };

    enum EventCategory
    {
        None = 0,
        EventCategoryApplication = ENG_BIT(0),
        EventCategoryInput = ENG_BIT(1),
        EventCategoryKeyboard = ENG_BIT(2),
        EventCategoryMouse = ENG_BIT(3),
        EventCategoryMouseButton = ENG_BIT(4)
    };

#define EVENT_CLASS_TYPE(type)                                                  \
    static EventType GetStaticType() { return EventType::type; }                \
    virtual EventType GetEventType() const override { return GetStaticType(); } \
    virtual const char *GetName() const override { return #type; }

#define EVENT_CLASS_CATEGORY(category) \
    virtual int GetCategoryFlags() const override { return category; }

    class Event
    {
    public:
        Event(Window *window)
            : m_Window(*window) {}

        virtual ~Event() = default;
        bool handled = false;

        virtual EventType GetEventType() const = 0;
        virtual const char *GetName() const = 0;
        virtual int GetCategoryFlags() const = 0;

        virtual std::string ToString() const { return GetName(); }

        inline bool IsInCategory(EventCategory category)
        {
            return GetCategoryFlags() & category;
        }

        Window &GetWindow() { return m_Window; }

    private:
        friend class EventDispatcher;
        Window &m_Window;
    };

    class EventDispatcher
    {
        template <typename T>
        using EventFn = std::function<bool(T &)>;

    public:
        EventDispatcher(Event &event)
            : m_Event(event)
        {
        }

        template <typename T, typename F>
        bool Dispatch(const F &func)
        {
            if (m_Event.GetEventType() == T::GetStaticType())
            {
                m_Event.handled = func(static_cast<T &>(m_Event));
                return true;
            }
            return false;
        }

    private:
        Event &m_Event;
    };

    inline std::ostream &operator<<(std::ostream &os, const Event &e)
    {
        return os << e.ToString();
    }
}