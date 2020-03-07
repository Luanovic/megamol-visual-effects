/*
 * Graph.cpp
 *
 * Copyright (C) 2019 by Universitaet Stuttgart (VIS).
 * Alle Rechte vorbehalten.
 */

#include "stdafx.h"
#include "Graph.h"


using namespace megamol;
using namespace megamol::gui::configurator;


int megamol::gui::configurator::Graph::generated_uid = 0; /// must be greater than or equal to zero


megamol::gui::configurator::Graph::Graph(const std::string& graph_name)
    : modules(), calls(), uid(this->generate_unique_id()), name(graph_name), dirty_flag(true), present() {}


megamol::gui::configurator::Graph::~Graph(void) {}


bool megamol::gui::configurator::Graph::AddModule(
    const ModuleStockVectorType& stock_modules, const std::string& module_class_name) {

    try {
        bool found = false;
        for (auto& mod : stock_modules) {
            if (module_class_name == mod.class_name) {
                auto mod_ptr = std::make_shared<Module>(this->generate_unique_id());
                mod_ptr->class_name = mod.class_name;
                mod_ptr->description = mod.description;
                mod_ptr->plugin_name = mod.plugin_name;
                mod_ptr->is_view = mod.is_view;
                // Generate unique name based on uid
                mod_ptr->name = mod.class_name + "_" + std::to_string(mod_ptr->uid);
                mod_ptr->name_space = "";
                mod_ptr->is_view_instance = false;

                for (auto& p : mod.parameters) {
                    Parameter param_slot(this->generate_unique_id(), p.type, p.storage, p.minval, p.maxval);
                    param_slot.full_name = p.full_name;
                    param_slot.description = p.description;

                    mod_ptr->parameters.emplace_back(param_slot);
                }

                for (auto& call_slots_type : mod.call_slots) {
                    for (auto& c : call_slots_type.second) {
                        CallSlot call_slot(this->generate_unique_id());
                        call_slot.name = c.name;
                        call_slot.description = c.description;
                        call_slot.compatible_call_idxs = c.compatible_call_idxs;
                        call_slot.type = c.type;

                        mod_ptr->AddCallSlot(std::make_shared<CallSlot>(call_slot));
                    }
                }

                for (auto& call_slot_type_list : mod_ptr->GetCallSlots()) {
                    for (auto& call_slot : call_slot_type_list.second) {
                        call_slot->ConnectParentModule(mod_ptr);
                    }
                }

                this->modules.emplace_back(mod_ptr);
                // vislib::sys::Log::DefaultLog.WriteInfo("Added module: %s [%s, %s, line %d]\n",
                //    mod_ptr->class_name.c_str(), __FILE__, __FUNCTION__, __LINE__);

                this->dirty_flag = true;
                return true;
            }
        }
    } catch (std::exception e) {
        vislib::sys::Log::DefaultLog.WriteError(
            "Error: %s [%s, %s, line %d]\n", e.what(), __FILE__, __FUNCTION__, __LINE__);
        return false;
    } catch (...) {
        vislib::sys::Log::DefaultLog.WriteError("Unknown Error. [%s, %s, line %d]\n", __FILE__, __FUNCTION__, __LINE__);
        return false;
    }

    vislib::sys::Log::DefaultLog.WriteError("Unable to find module in stock: %s [%s, %s, line %d]\n",
        module_class_name.c_str(), __FILE__, __FUNCTION__, __LINE__);
    return false;
}


bool megamol::gui::configurator::Graph::DeleteModule(int module_uid) {

    try {
        for (auto iter = this->modules.begin(); iter != this->modules.end(); iter++) {
            if ((*iter)->uid == module_uid) {
                (*iter)->RemoveAllCallSlots();

                // vislib::sys::Log::DefaultLog.WriteWarn("Found %i references pointing to module. [%s, %s, line %d]\n",
                //    (*iter).use_count(), __FILE__, __FUNCTION__, __LINE__);
                assert((*iter).use_count() == 1);
                (*iter).reset();
                this->modules.erase(iter);
                this->DeleteDisconnectedCalls();
                vislib::sys::Log::DefaultLog.WriteInfo("Deleted module: %s [%s, %s, line %d]\n",
                   (*iter)->class_name.c_str(), __FILE__, __FUNCTION__, __LINE__);

                this->dirty_flag = true;
                return true;
            }
        }

    } catch (std::exception e) {
        vislib::sys::Log::DefaultLog.WriteError(
            "Error: %s [%s, %s, line %d]\n", e.what(), __FILE__, __FUNCTION__, __LINE__);
        return false;
    } catch (...) {
        vislib::sys::Log::DefaultLog.WriteError("Unknown Error. [%s, %s, line %d]\n", __FILE__, __FUNCTION__, __LINE__);
        return false;
    }

    vislib::sys::Log::DefaultLog.WriteWarn("Invalid module uid. [%s, %s, line %d]\n", __FILE__, __FUNCTION__, __LINE__);
    return false;
}


bool megamol::gui::configurator::Graph::AddCall(const CallStockVectorType& stock_calls,
    const std::string& call_class_name, CallSlotPtrType call_slot_1, CallSlotPtrType call_slot_2) {

    try {
        bool found_stock_call = false;
        Call::StockCall call_stock_data;
        for (auto& csd : stock_calls) {
            if (csd.class_name == call_class_name) {
                call_stock_data = csd;
                found_stock_call = true;
            }
        }
        if (!found_stock_call) {
            vislib::sys::Log::DefaultLog.WriteWarn("Unable to find call: %s [%s, %s, line %d]\n",
                call_class_name.c_str(), __FILE__, __FUNCTION__, __LINE__);
            return false;
        }

        auto call_ptr = std::make_shared<Call>(this->generate_unique_id());
        call_ptr->class_name = call_stock_data.class_name;
        call_ptr->description = call_stock_data.description;
        call_ptr->plugin_name = call_stock_data.plugin_name;
        call_ptr->functions = call_stock_data.functions;

        if (call_ptr->ConnectCallSlots(call_slot_1, call_slot_2) && call_slot_1->ConnectCall(call_ptr) &&
            call_slot_2->ConnectCall(call_ptr)) {

            this->calls.emplace_back(call_ptr);
            // vislib::sys::Log::DefaultLog.WriteInfo("Added call: %s [%s, %s, line %d]\n",
            //    call_ptr->class_name.c_str(), __FILE__, __FUNCTION__, __LINE__);

            this->dirty_flag = true;
        } else {
            this->DeleteCall(call_ptr->uid);
            vislib::sys::Log::DefaultLog.WriteWarn("Unable to connect call: %s [%s, %s, line %d]\n",
                call_ptr->class_name.c_str(), __FILE__, __FUNCTION__, __LINE__);
            return false;
        }

    } catch (std::exception e) {
        vislib::sys::Log::DefaultLog.WriteError(
            "Error: %s [%s, %s, line %d]\n", e.what(), __FILE__, __FUNCTION__, __LINE__);
        return false;
    } catch (...) {
        vislib::sys::Log::DefaultLog.WriteError("Unknown Error. [%s, %s, line %d]\n", __FILE__, __FUNCTION__, __LINE__);
        return false;
    }

    return true;
}

bool megamol::gui::configurator::Graph::DeleteDisconnectedCalls(void) {

    try {
        // Create separate uid list to avoid iterator conflict when operating on calls list while deleting.
        std::vector<int> call_uids;
        for (auto& call : this->calls) {
            if (!call->IsConnected()) {
                call_uids.emplace_back(call->uid);
            }
        }
        for (auto& id : call_uids) {
            this->DeleteCall(id);
            this->dirty_flag = true;
        }
    } catch (std::exception e) {
        vislib::sys::Log::DefaultLog.WriteError(
            "Error: %s [%s, %s, line %d]\n", e.what(), __FILE__, __FUNCTION__, __LINE__);
        return false;
    } catch (...) {
        vislib::sys::Log::DefaultLog.WriteError("Unknown Error. [%s, %s, line %d]\n", __FILE__, __FUNCTION__, __LINE__);
        return false;
    }

    return true;
}


bool megamol::gui::configurator::Graph::DeleteCall(int call_uid) {

    try {
        for (auto iter = this->calls.begin(); iter != this->calls.end(); iter++) {
            if ((*iter)->uid == call_uid) {
                (*iter)->DisConnectCallSlots();

                // vislib::sys::Log::DefaultLog.WriteWarn("Found %i references pointing to call. [%s, %s, line %d]\n",
                //    (*iter).use_count(), __FILE__, __FUNCTION__, __LINE__);
                assert((*iter).use_count() == 1);
                (*iter).reset();
                this->calls.erase(iter);
                vislib::sys::Log::DefaultLog.WriteInfo("Deleted call: %s [%s, %s, line %d]\n",
                    (*iter)->class_name.c_str(), __FILE__, __FUNCTION__, __LINE__);

                this->dirty_flag = true;
                return true;
            }
        }
    } catch (std::exception e) {
        vislib::sys::Log::DefaultLog.WriteError(
            "Error: %s [%s, %s, line %d]\n", e.what(), __FILE__, __FUNCTION__, __LINE__);
        return false;
    } catch (...) {
        vislib::sys::Log::DefaultLog.WriteError("Unknown Error. [%s, %s, line %d]\n", __FILE__, __FUNCTION__, __LINE__);
        return false;
    }

    vislib::sys::Log::DefaultLog.WriteWarn("Invalid call uid. [%s, %s, line %d]\n", __FILE__, __FUNCTION__, __LINE__);
    return false;
}


// GRAPH PRESENTATION ####################################################

megamol::gui::configurator::Graph::Presentation::Presentation(void)
    : font(nullptr)
    , utils()
    , canvas_position(ImVec2(0.0f, 0.0f))
    , canvas_size(ImVec2(1.0f, 1.0f))
    , canvas_scrolling(ImVec2(0.0f, 0.0f))
    , canvas_zooming(1.0f)
    , canvas_offset(ImVec2(0.0f, 0.0f))
    , show_grid(false)
    , show_call_names(true)
    , show_slot_names(true)
    , show_module_names(true)
    , selected_module_uid(GUI_INVALID_ID)
    , selected_call_uid(GUI_INVALID_ID)

    , selected_slot_ptr(nullptr)//
    , process_selected_slot(0)//

    , layout_current_graph(true)
    , split_width(-1.0f) // !
    , mouse_wheel(0.0f)
    , params_visible(true)
    , params_readonly(false)
    , param_name_space()
    , param_present(Parameter::Presentations::DEFAULT) {}


megamol::gui::configurator::Graph::Presentation::~Presentation(void) {}


bool megamol::gui::configurator::Graph::Presentation::GUI_Present(megamol::gui::configurator::Graph& graph,
    float child_width, ImFont* graph_font, megamol::gui::HotKeyArrayType& hotkeys, bool& delete_graph) {

    bool retval = false;
    this->font = graph_font;
    bool rename_popup_open = false;

    try {

        if (ImGui::GetCurrentContext() == nullptr) {
            vislib::sys::Log::DefaultLog.WriteError(
                "No ImGui context available. [%s, %s, line %d]\n", __FILE__, __FUNCTION__, __LINE__);
            return false;
        }

        ImGuiIO& io = ImGui::GetIO();

        ImGui::PushID(graph.GetUID());

        // Tab showing one graph
        ImGuiTabItemFlags tab_flags = ImGuiTabItemFlags_None;
        if (graph.IsDirty()) {
            tab_flags |= ImGuiTabItemFlags_UnsavedDocument;
        }
        std::string graph_label = "    " + graph.GetName() + "  ###graph" + std::to_string(graph.GetUID());
        bool open = true;
        if (ImGui::BeginTabItem(graph_label.c_str(), &open, tab_flags)) {

            // Context menu
            if (ImGui::BeginPopupContextItem()) {
                if (ImGui::MenuItem("Rename")) {
                    rename_popup_open = true;
                }
                ImGui::EndPopup();
            }

            // Process module deletion
            if (std::get<1>(hotkeys[HotkeyIndex::DELETE_GRAPH_ITEM])) {
                std::get<1>(hotkeys[HotkeyIndex::DELETE_GRAPH_ITEM]) = false;
                this->selected_slot_ptr = nullptr; //!
                if (this->selected_module_uid > 0) {
                    graph.DeleteModule(this->selected_module_uid);
                }
                if (this->selected_call_uid > 0) {
                    graph.DeleteCall(this->selected_call_uid);
                }
            }

            // Register trigger for connecting call
            if ((this->selected_slot_ptr != nullptr) && (io.MouseReleased[0])) {
                this->process_selected_slot = 2;
            }

            // Update positions and sizes
            if (this->layout_current_graph) {
                this->layout_graph(graph);
                this->layout_current_graph = false;
            }

            // Draw
            this->menu(graph);

            if (this->selected_module_uid > 0) {
                // One time init depending on available window width
                if (this->split_width < 0.0f) {
                    this->split_width = ImGui::GetWindowWidth() * 0.75f;
                }
                float child_width_auto = 0.0f;
                this->utils.VerticalSplitter(&this->split_width, &child_width_auto);

                this->canvas(graph, this->split_width, hotkeys);
                ImGui::SameLine();
                this->parameters(graph, child_width_auto, hotkeys);
            } else {
                this->canvas(graph, child_width, hotkeys);
            }

            retval = true;
            ImGui::EndTabItem();
        }

        // Set delete flag if tab was closed
        if (!open) delete_graph = true;

        // Rename pop-up
        this->utils.RenamePopUp("Rename Project", rename_popup_open, graph.GetName());

        ImGui::PopID();
    } catch (std::exception e) {
        vislib::sys::Log::DefaultLog.WriteError(
            "Error: %s [%s, %s, line %d]\n", e.what(), __FILE__, __FUNCTION__, __LINE__);
        return false;
    } catch (...) {
        vislib::sys::Log::DefaultLog.WriteError("Unknown Error. [%s, %s, line %d]\n", __FILE__, __FUNCTION__, __LINE__);
        return false;
    }

    return retval;
}


void megamol::gui::configurator::Graph::Presentation::menu(megamol::gui::configurator::Graph& graph) {

    const float child_height = ImGui::GetItemsLineHeightWithSpacing() * 1.0f;
    const auto child_flags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove;

    ImGui::BeginChild("graph_menu", ImVec2(0.0f, child_height), false, child_flags);

    // Main View Checkbox
    ModulePtrType selected_mod_ptr = nullptr;
    if (this->selected_module_uid != GUI_INVALID_ID) {
        for (auto& mod : graph.GetGraphModules()) {
            if ((this->selected_module_uid == mod->uid) && (mod->is_view)) {
                selected_mod_ptr = mod;
            }
        }
    }
    if (selected_mod_ptr == nullptr) {
        ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
        bool checked = false;
        ImGui::Checkbox("Main View", &checked);
        ImGui::PopItemFlag();
        ImGui::PopStyleVar();
    }
    else {
        if (ImGui::Checkbox("Main View", &selected_mod_ptr->is_view_instance)) {
            if (selected_mod_ptr->is_view_instance) {
                // Set all other modules to non main views
                for (auto& mod : graph.GetGraphModules()) {
                    if (this->selected_module_uid != mod->uid) {
                        mod->is_view_instance = false;
                    }
                }
            }
        }
    }
    ImGui::SameLine();

    if (ImGui::Button("Reset###reset_scrolling")) {
        this->canvas_scrolling = ImVec2(0.0f, 0.0f);
    }
    ImGui::SameLine();

    ImGui::Text("Scrolling: %.4f,%.4f", this->canvas_scrolling.x, this->canvas_scrolling.y);
    this->utils.HelpMarkerToolTip("Middle Mouse Button");

    ImGui::SameLine();

    if (ImGui::Button("Reset###reset_zooming")) {
        this->canvas_zooming = 1.0f;
    }
    ImGui::SameLine();

    ImGui::Text("Zooming: %.4f", this->canvas_zooming);
    this->utils.HelpMarkerToolTip("Mouse Wheel");

    ImGui::SameLine();

    ImGui::Checkbox("Grid", &this->show_grid);

    ImGui::SameLine();

    if (ImGui::Checkbox("Call Names", &this->show_call_names)) {
        for (auto& call : graph.GetGraphCalls()) {
            call->GUI_SetLabelVisibility(this->show_call_names);
        }
    }
    ImGui::SameLine();

    if (ImGui::Checkbox("Module Names", &this->show_module_names)) {
        for (auto& mod : graph.GetGraphModules()) {
            mod->GUI_SetLabelVisibility(this->show_module_names);
        }
    }
    ImGui::SameLine();

    if (ImGui::Checkbox("Slot Names", &this->show_slot_names)) {
        for (auto& mod : graph.GetGraphModules()) {
            for (auto& call_slot_types : mod->GetCallSlots()) {
                for (auto& call_slots : call_slot_types.second) {
                    call_slots->GUI_SetLabelVisibility(this->show_slot_names);
                }
            }
        }
    }
    ImGui::SameLine();

    if (ImGui::Button("Layout Graph")) {
        this->layout_current_graph = true;
    }

    ImGui::EndChild();
}


void megamol::gui::configurator::Graph::Presentation::canvas(
    megamol::gui::configurator::Graph& graph, float child_width, megamol::gui::HotKeyArrayType& hotkeys) {

    ImGuiIO& io = ImGui::GetIO();

    // Font scaling is applied next frame after ImGui::Begin()
    // Font for graph should not be the currently used font of the gui.
    if (this->font == nullptr) {
        vislib::sys::Log::DefaultLog.WriteError(
            "Found no font for configurator. Provide font via GuiView::SetGraphFont(). [%s, %s, line %d]\n", __FILE__,
            __FUNCTION__, __LINE__);
        return;
    }
    ImGui::PushFont(this->font);
    ImGui::GetFont()->Scale = this->canvas_zooming;

    const ImU32 COLOR_CANVAS_BACKGROUND = IM_COL32(75, 75, 75, 255);

    ImGui::PushStyleColor(ImGuiCol_ChildBg, COLOR_CANVAS_BACKGROUND);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(1, 1));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

    ImGui::BeginChild(
        "region", ImVec2(child_width, 0.0f), true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove);
    this->canvas_position = ImGui::GetCursorScreenPos();
    this->canvas_size = ImGui::GetWindowSize();

    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    assert(draw_list != nullptr);
    draw_list->ChannelsSplit(2);

    if (ImGui::IsMouseClicked(0) && ImGui::IsWindowHovered()) {
        this->selected_module_uid = GUI_INVALID_ID;
        this->selected_call_uid = GUI_INVALID_ID;
        this->selected_slot_ptr = nullptr;
    }

    // Display grid -------------------
    if (this->show_grid) {
        this->canvas_grid(graph);
    }
    ImGui::PopStyleVar(2);

    // Draw modules -------------------
    for (auto& mod : graph.GetGraphModules()) {
        auto id = mod->GUI_Present(this->canvas_offset, this->canvas_zooming, hotkeys);
        if (id != GUI_INVALID_ID) {
            this->selected_module_uid = id;
        }
    }

    // Draw calls ---------------------
    for (auto& call : graph.GetGraphCalls()) {
        auto id = call->GUI_Present(this->canvas_offset, this->canvas_zooming, hotkeys);
        if (id != GUI_INVALID_ID) {
            this->selected_call_uid = id;
        }
    }

    // Draw dragged call --------------
    this->canvas_dragged_call(graph);

    // Zooming and Scaling  -----------
    /// Must be checked inside canvas child window.
    if (ImGui::IsWindowHovered() && !ImGui::IsAnyItemActive()) {

        // Scrolling (2 = Middle Mouse Button)
        if (ImGui::IsMouseDragging(2, 0.0f)) {
            this->canvas_scrolling = this->canvas_scrolling + ImGui::GetIO().MouseDelta / this->canvas_zooming;
        }

        // Zooming (Mouse Wheel)
        if (this->mouse_wheel != io.MouseWheel) {
            const float factor = (30.0f / this->canvas_zooming);
            float last_zooming = this->canvas_zooming;
            this->canvas_zooming = this->canvas_zooming + io.MouseWheel / factor;
            // Limit zooming
            this->canvas_zooming = (this->canvas_zooming < 0.0f) ? 0.000001f : (this->canvas_zooming);
            if (this->canvas_zooming > 0.0f) {
                // Compensate zooming shift of origin
                ImVec2 scrolling_diff =
                    (this->canvas_scrolling * last_zooming) - (this->canvas_scrolling * this->canvas_zooming);
                this->canvas_scrolling += (scrolling_diff / this->canvas_zooming);
                // Move origin away from mouse position
                ImVec2 current_mouse_pos = this->canvas_offset - ImGui::GetMousePos();
                ImVec2 new_mouse_position = (current_mouse_pos / last_zooming) * this->canvas_zooming;
                this->canvas_scrolling += ((new_mouse_position - current_mouse_pos) / this->canvas_zooming);
            }
        }
        this->mouse_wheel = io.MouseWheel;
    }
    this->canvas_offset = this->canvas_position + (this->canvas_scrolling * this->canvas_zooming);

    /// DEBUG Draw point at origin
    draw_list->AddCircleFilled(this->canvas_offset, 10.0f * this->canvas_zooming, IM_COL32(192, 0, 0, 255));

    draw_list->ChannelsMerge();
    ImGui::EndChild();
    ImGui::PopStyleColor();

    if (this->process_selected_slot > 0) {
        this->process_selected_slot--;
    }

    // Reset font
    ImGui::PopFont();
}

void megamol::gui::configurator::Graph::Presentation::parameters(
    megamol::gui::configurator::Graph& graph, float child_width, HotKeyArrayType& hotkeys) {

    ImGui::BeginGroup();

    float param_child_height = ImGui::GetItemsLineHeightWithSpacing() * 2.25f;
    auto child_flags = ImGuiWindowFlags_AlwaysUseWindowPadding | ImGuiWindowFlags_NoScrollbar;

    ImGui::BeginChild("parameter_search_child", ImVec2(child_width, param_child_height), false, child_flags);

    ImGui::Text("Parameters");
    ImGui::Separator();

    if (std::get<1>(hotkeys[HotkeyIndex::PARAMETER_SEARCH])) {
        std::get<1>(hotkeys[HotkeyIndex::PARAMETER_SEARCH]) = false;
        this->utils.SetSearchFocus(true);
    }
    std::string help_text = "[" + std::get<0>(hotkeys[HotkeyIndex::PARAMETER_SEARCH]).ToString() +
                            "] Set keyboard focus to search input field.\n"
                            "Case insensitive substring search in parameter names.";
    this->utils.StringSearch("graph_parameter_search", help_text);
    auto search_string = this->utils.GetSearchString();

    ImGui::EndChild();

    // Get pointer to currently selected module
    ModulePtrType modptr;
    for (auto& mod : graph.GetGraphModules()) {
        if (mod->uid == this->selected_module_uid) {
            modptr = mod;
        }
    }
    if (modptr != nullptr) {

        float param_child_height = ImGui::GetItemsLineHeightWithSpacing() * 2.25f;
        ImGui::BeginChild("parameter_info_child", ImVec2(child_width, param_child_height), false, child_flags);

        ImGui::Separator();

        ImGui::Text("Selected Module:");
        ImGui::SameLine();
        ImGui::TextColored(ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive), modptr->name.c_str());

        ImGui::Text("Options:");
        ImGui::SameLine();

        // Visibility
        if (ImGui::Checkbox("Visibility", &this->params_visible)) {
            for (auto& param : modptr->parameters) {
                param.GUI_SetLabelVisibility(this->params_visible);
            }
        }
        ImGui::SameLine();

        // Read-only option
        if (ImGui::Checkbox("Read-Only", &this->params_readonly)) {
            for (auto& param : modptr->parameters) {
                param.GUI_SetReadOnly(this->params_readonly);
            }
        }
        ImGui::SameLine();

        // Presentations
        if (Parameter::GUI_PresentationButton(this->param_present, "Presentation")) {
            for (auto& param : modptr->parameters) {
                param.GUI_SetPresentation(this->param_present);
            }
        }
        ImGui::EndChild();

        auto child_flags = ImGuiWindowFlags_AlwaysVerticalScrollbar | ImGuiWindowFlags_HorizontalScrollbar;
        ImGui::BeginChild("parameter_list_child", ImVec2(child_width, 0.0f), true, child_flags);

        bool param_name_space_open = true;
        unsigned int param_indent_stack = 0;

        for (auto& param : modptr->parameters) {
            // Filter module by given search string
            bool search_filter = true;
            if (!search_string.empty()) {
                search_filter = this->utils.FindCaseInsensitiveSubstring(param.full_name, search_string);
            }

            // Add Collapsing header depending on parameter namespace
            std::string current_param_namespace = param.GetNameSpace();
            if (current_param_namespace != this->param_name_space) {
                this->param_name_space = current_param_namespace;
                while (param_indent_stack > 0) {
                    param_indent_stack--;
                    ImGui::Unindent();
                }

                ImGui::Separator();
                if (!this->param_name_space.empty()) {
                    ImGui::Indent();
                    std::string label = this->param_name_space + "###" + param.full_name;
                    // Open all namespace headers when parameter search is active
                    if (!search_string.empty()) {
                        auto headerId = ImGui::GetID(label.c_str());
                        ImGui::GetStateStorage()->SetInt(headerId, 1);
                    }
                    param_name_space_open = ImGui::CollapsingHeader(label.c_str(), ImGuiTreeNodeFlags_DefaultOpen);
                    param_indent_stack++;
                } else {
                    param_name_space_open = true;
                }
            }

            // Draw parameter
            if (search_filter && param_name_space_open) {
                param.GUI_Present();
            }
        }


        ImGui::EndChild();
    }

    ImGui::EndGroup();
}


void megamol::gui::configurator::Graph::Presentation::canvas_grid(megamol::gui::configurator::Graph& graph) {

    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    assert(draw_list != nullptr);

    draw_list->ChannelsSetCurrent(0); // Background

    const ImU32 COLOR_GRID = IM_COL32(192, 192, 192, 40);
    const float GRID_SIZE = 64.0f * this->canvas_zooming;

    ImVec2 relative_offset = this->canvas_offset - this->canvas_position;

    for (float x = fmodf(relative_offset.x, GRID_SIZE); x < this->canvas_size.x; x += GRID_SIZE) {
        draw_list->AddLine(ImVec2(x, 0.0f) + this->canvas_position,
            ImVec2(x, this->canvas_size.y) + this->canvas_position, COLOR_GRID);
    }

    for (float y = fmodf(relative_offset.y, GRID_SIZE); y < this->canvas_size.y; y += GRID_SIZE) {
        draw_list->AddLine(ImVec2(0.0f, y) + this->canvas_position,
            ImVec2(this->canvas_size.x, y) + this->canvas_position, COLOR_GRID);
    }
}


void megamol::gui::configurator::Graph::Presentation::canvas_dragged_call(megamol::gui::configurator::Graph& graph) {

    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    assert(draw_list != nullptr);
    draw_list->ChannelsSetCurrent(0); // Background

    const auto COLOR_CALL_CURVE = IM_COL32(128, 128, 0, 255);

    const float CURVE_THICKNESS = 3.0f;

    if (this->selected_slot_ptr != nullptr) {
        ImVec2 current_pos = ImGui::GetMousePos();
        bool mouse_inside_canvas = false;

        if ((current_pos.x >= this->canvas_position.x) &&
            (current_pos.x <= (this->canvas_position.x + this->canvas_size.x)) &&
            (current_pos.y >= this->canvas_position.y) &&
            (current_pos.y <= (this->canvas_position.y + this->canvas_size.y))) {
            mouse_inside_canvas = true;
        }
        if (ImGui::IsMouseDown(0) && mouse_inside_canvas) {
            ImVec2 p1 = this->selected_slot_ptr->GUI_GetPosition();
            ImVec2 p2 = ImGui::GetMousePos();
            if (this->selected_slot_ptr->type == CallSlot::CallSlotType::CALLEE) {
                ImVec2 tmp = p1;
                p1 = p2;
                p2 = tmp;
            }
            draw_list->AddBezierCurve(p1, p1 + ImVec2(+50, 0), p2 + ImVec2(-50, 0), p2, COLOR_CALL_CURVE,
                CURVE_THICKNESS * this->canvas_zooming);
        }
    }
}


bool megamol::gui::configurator::Graph::Presentation::layout_graph(
    megamol::gui::configurator::Graph& graph) {

/*
    // Really simple layouting sorting modules into differnet layers
    std::vector<std::vector<graph::ModulePtrType>> layers;
    layers.clear();

    // Fill first layer with modules having no connected callee
    // (Cycles are ignored)
    layers.emplace_back();
    for (auto& mod : graph->GetGraphModules()) {
        bool any_connected_callee = false;
        for (auto& callee_slot : mod->GetCallSlots(graph::CallSlot::CallSlotType::CALLEE)) {
            if (callee_slot->CallsConnected()) {
                any_connected_callee = true;
            }
        }
        if (!any_connected_callee) {
            layers.back().emplace_back(mod);
        }
    }

    // Loop while modules are added to new layer.
    bool added_module = true;
    while (added_module) {
        added_module = false;
        // Add new layer
        layers.emplace_back();
        // Loop through last filled layer
        for (auto& mod : layers[layers.size() - 2]) {
            for (auto& caller_slot : mod->GetCallSlots(graph::CallSlot::CallSlotType::CALLER)) {
                if (caller_slot->CallsConnected()) {
                    for (auto& call : caller_slot->GetConnectedCalls()) {
                        auto add_mod = call->GetCallSlot(graph::CallSlot::CallSlotType::CALLEE)->GetParentModule();
                        // Check if module was already added
                        bool found_module = false;
                        for (auto& layer : layers) {
                            for (auto& m : layer) {
                                if (m == add_mod) {
                                    found_module = true;
                                }
                            }
                        }
                        if (!found_module) {
                            layers.back().emplace_back(add_mod);
                            added_module = true;
                        }
                    }
                }
            }
        }
    }

    // Calculate new positions of modules
    const float border_offset = this->slot_radius * 4.0f;
    ImVec2 init_position = ImVec2(-1.0f * this->canvas_scrolling.x, -1.0f * this->canvas_scrolling.y);
    ImVec2 pos = init_position;
    float max_call_width = 25.0f;
    float max_module_width = 0.0f;
    size_t layer_mod_cnt = 0;
    for (auto& layer : layers) {
        if (this->show_call_names) {
            max_call_width = 0.0f;
        }
        max_module_width = 0.0f;
        layer_mod_cnt = layer.size();
        pos.x += border_offset;
        pos.y = init_position.y + border_offset;
        for (int i = 0; i < layer_mod_cnt; i++) {
            auto mod = layer[i];
            if (this->show_call_names) {
                for (auto& caller_slot : mod->GetCallSlots(graph::CallSlot::CallSlotType::CALLER)) {
                    if (caller_slot->CallsConnected()) {
                        for (auto& call : caller_slot->GetConnectedCalls()) {
                            auto call_name_length = this->utils.TextWidgetWidth(call->class_name);
                            max_call_width =
                                (call_name_length > max_call_width) ? (call_name_length) : (max_call_width);
                        }
                    }
                }
            }
            mod->present.position = pos;
            pos.y += mod->present.size.y + border_offset;
            max_module_width = (mod->present.size.x > max_module_width) ? (mod->present.size.x) :
(max_module_width);
        }
        pos.x += (max_module_width + max_call_width + border_offset);
    }
*/
    return true;
}
