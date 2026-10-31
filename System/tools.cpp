//
//  tools.cpp
//  Mandarine
//
//  Created by Jarrod Norwell on 17/10/2026.
//

#if __APPLE__
#include <TargetConditionals.h>
#endif

#if TARGET_OS_IPHONE

#include "avocado/config.h"
#include "avocado/system.h"
#include "avocado/tools.h"
#include "avocado/disc/load.h"
#include "avocado/memory_card/card_formats.h"
#include "avocado/sound/sound.h"
#include "avocado/state/state.h"
#include "avocado/utils/event.h"
#include "avocado/utils/file.h"
#include "avocado/utils/gpu_draw_list.h"
#include "avocado/utils/psf.h"

#else

#include "config.h"
#include "system.h"
#include "tools.h"
#include "disc/load.h"
#include "memory_card/card_formats.h"
#include "sound/sound.h"
#include "state/state.h"
#include "utils/event.h"
#include "utils/file.h"
#include "utils/gpu_draw_list.h"
#include "utils/psf.h"

#endif // TARGET_OS_IPHONE

#include <fmt/core.h>
#include <fmt/format.h>
#include <fmt/format-inl.h>


void tools::bootstrap(std::unique_ptr<System>& system) {
    sound::clear();

    system = tools::reset();
    system->cpu->addBreakpoint(0x80030000);

    while (system->state == System::State::run)
        system->emulateFrame();
}

std::unique_ptr<System> tools::reset() {
    auto system = std::make_unique<System>();

    std::string bios = config.bios;
    if (!bios.empty() && system->load(bios))
        fmt::print("[INFO] Using bios {}\n", getFilenameExt(bios));

        std::string extension = config.extension;
        if (!extension.empty() && system->load(getFileContents(extension)))
            fmt::print("[INFO] Using extension {}\n", getFilenameExt(extension));

        std::string iso = config.iso;
        if (!iso.empty())
            tools::load(system, iso);

        tools::handle_memory_card<false>(system, 0);
        tools::handle_memory_card<false>(system, 1);

        return system;
}


void tools::load(std::unique_ptr<System>& system, const std::string& path) {
    std::string ext = getExtension(path);
    std::transform(ext.begin(), ext.end(), ext.begin(), tolower);

    std::string filenameExt = getFilenameExt(path);
    std::transform(filenameExt.begin(), filenameExt.end(), filenameExt.begin(), tolower);

    if (ext == "psf" || ext == "minipsf") {
        bootstrap(system);
        if (loadPsf(system.get(), path))
            toast(fmt::format("{} loaded", filenameExt));
        else
            toast(fmt::format("Cannot load {}", filenameExt));
        system->state = System::State::run;
        return;
    }

    if (ext == "exe" || ext == "psexe") {
        bool isPaused = system->state == System::State::pause;
        bootstrap(system);
        if (system->load(getFileContents(path), true))
            toast(fmt::format("{} loaded", filenameExt));
        else
            toast(fmt::format("Cannot load {}", filenameExt));

        system->state = isPaused ? System::State::pause : System::State::run;
        return;
    }

    if (ext == "state")
        if (state::loadFromFile(system.get(), path)) {
            system->state = System::State::pause;
            return;
        }

    if (ext == "gpudrawlist") {
        if (GpuDrawList::load(system.get(), path)) {
            system->state = System::State::pause;
            GpuDrawList::replayCommands(system->gpu.get());
            toast(fmt::format("{} loaded", filenameExt));
            listener.getBus()->postpone(Event::Gui::Debug::OpenDrawListWindows{});
            listener.getBus()->process();
            return;
        }
    }

    std::unique_ptr<disc::Disc> disc = disc::load(path);
    if (disc) {
        system->cdrom->setShell(true);
        system->cdrom->disc = std::move(disc);
        system->cdrom->setShell(false);
        fmt::print("{}", "loaded");
        toast(fmt::format("{} loaded", filenameExt));
    } else {
        fmt::print("{}", "cannot load");
        toast(fmt::format("Cannot load {}", filenameExt));
    }
}


template <bool write>
bool tools::handle_memory_card(std::unique_ptr<System>& system , const int slot, const bool force) {
    if (write) {
        if (!force && !system->controller->card[slot]->dirty)
            return true;

        std::string pathCard = config.memoryCard[slot].path;

        if (pathCard.empty())
            return false;

        if (!memory_card::save(system->controller->card[slot]->data, pathCard))
            return false;

        system->controller->card[slot]->dirty = false;

        return true;
    } else {
        auto card = system->controller->card[slot].get();
        card->inserted = false;

        auto path = config.memoryCard[slot].path;
        if (path.empty())
            return false;

        auto cardData = memory_card::load(path);
        if (!cardData)
            return false;

        std::copy(cardData->begin(), cardData->end(), system->controller->card[slot]->data.begin());
        card->inserted = true;
        card->setFresh();

        return true;
    }
}
template bool tools::handle_memory_card<true>(std::unique_ptr<System>&, const int, const bool);
