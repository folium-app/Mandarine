#include "utils/event.h"

auto bus = std::make_shared<dexode::EventBus>();
dexode::EventBus::Listener listener{bus};

void toast(const std::string &message)
{
    bus->postpone(Event::Gui::Toast{message});
    bus->process();
}
