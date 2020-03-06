/*
 * Module.h
 *
 * Copyright (C) 2019 by Universitaet Stuttgart (VIS).
 * Alle Rechte vorbehalten.
 */

#ifndef MEGAMOL_GUI_GRAPH_MODULE_H_INCLUDED
#define MEGAMOL_GUI_GRAPH_MODULE_H_INCLUDED


#include "vislib/sys/Log.h"

#include <map>
#include <vector>

#include "CallSlot.h"
#include "GUIUtils.h"
#include "Parameter.h"


namespace megamol {
namespace gui {
namespace configurator {


// Forward declaration
class Call;
class CallSlot;
class Module;

// Pointer types to classes
typedef std::shared_ptr<Call> CallPtrType;
typedef std::shared_ptr<CallSlot> CallSlotPtrType;
typedef std::shared_ptr<Module> ModulePtrType;

/**
 * Defines module data structure for graph.
 */
class Module {
public:
    struct StockModule {
        std::string class_name;
        std::string description;
        std::string plugin_name;
        bool is_view;
        std::vector<Parameter::StockParameter> parameters;
        std::map<CallSlot::CallSlotType, std::vector<CallSlot::StockCallSlot>> call_slots;
    };

    enum Presentations : size_t { DEFAULT = 0, _COUNT_ = 1 };

    Module(int uid);
    ~Module();

    const int uid;

    std::string class_name;
    std::string description;
    std::string plugin_name;
    bool is_view;
    std::vector<Parameter> parameters;

    std::string name;
    std::string name_space;
    bool is_view_instance;

    bool AddCallSlot(CallSlotPtrType call_slot);
    bool RemoveAllCallSlots(void);
    const std::vector<CallSlotPtrType>& GetCallSlots(CallSlot::CallSlotType type);
    const std::map<CallSlot::CallSlotType, std::vector<CallSlotPtrType>>& GetCallSlots(void);

    const std::string FullName(void) const { return std::string(this->name_space + "::" + this->name); }

    // GUI Presentation -------------------------------------------------------

    ImGuiID GUI_Present(ImVec2 canvas_offset, float canvas_zooming, HotKeyArrayType& hotkeys) {
        return this->present.Present(*this, canvas_offset, canvas_zooming, hotkeys);
    }

    void GUI_SetLabelVisibility(bool visible) { this->present.label_visible = visible; }
    void GUI_SetPresentation(Module::Presentations present) { this->present.presentations = present; }
    void GUI_SetPosition(ImVec2 pos) { this->present.SetPosition(pos); }
    ImVec2 GUI_GetPosition(void) { return this->present.GetPosition(); }
    ImVec2 GUI_GetSize(void) { return this->present.GetSize(); }

private:
    std::map<CallSlot::CallSlotType, std::vector<CallSlotPtrType>> call_slots;

    /**
     * Defines GUI module presentation.
     */
    class Presentation {
    public:
        Presentation(void);

        ~Presentation(void);

        ImGuiID Present(Module& mod, ImVec2 canvas_offset, float canvas_zooming, HotKeyArrayType& hotkeys);

        void SetPosition(ImVec2 pos) { this->position = pos; }

        ImVec2 GetPosition(void) { return this->position; }
        ImVec2 GetSize(void) { return this->size; }

        Module::Presentations presentations;
        bool label_visible;

    private:
        ImVec2 position;
        ImVec2 size;
        std::string class_label;
        std::string name_label;
        GUIUtils utils;

    } present;
};


} // namespace configurator
} // namespace gui
} // namespace megamol

#endif // MEGAMOL_GUI_GRAPH_MODULE_H_INCLUDED