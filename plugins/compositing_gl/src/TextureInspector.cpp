/**
 * MegaMol
 * Copyright (c) 2022, MegaMol Dev Team
 * All rights reserved.
 */

#include "TextureInspector.h"
#include "compositing_gl/CompositingCalls.h"
#include "mmcore/param/BoolParam.h"

#include "imgui_tex_inspect.h"
#include "tex_inspect_opengl.h"

using namespace megamol::compositing_gl;


TextureInspector::TextureInspector()
    : megamol::core::Module()
    , get_data_slot_("getData", "Slot to fetch data")
    , output_tex_slot_("OutputTexture", "Gives access to the resulting output texture")
    , show_inspector_("On//Off", "Toggles the Imgui window on or off")
{
    core::param::BoolParam* bp = new core::param::BoolParam(true);
    this->show_inspector_ << bp;
    this->MakeSlotAvailable(&this->show_inspector_);

    this->output_tex_slot_.SetCallback(
        compositing::CallTexture2D::ClassName(), "GetData", &TextureInspector::getDataCallback);
    this->output_tex_slot_.SetCallback(
        compositing::CallTexture2D::ClassName(), "GetMetaData", &TextureInspector::getMetaDataCallback);
    this->MakeSlotAvailable(&this->output_tex_slot_);

    this->get_data_slot_.SetCompatibleCall<compositing::CallTexture2DDescription>();
    this->MakeSlotAvailable(&this->get_data_slot_);
}

TextureInspector::~TextureInspector() {
    this->Release();
}

bool TextureInspector::create() {
    ImGuiTexInspect::ImplOpenGL3_Init();
    ImGuiTexInspect::Init();
    ImGuiTexInspect::CreateContext();

    return true;
}

void TextureInspector::release() {}

bool TextureInspector::getDataCallback(core::Call& caller) {
    auto lhs_tc = dynamic_cast<compositing::CallTexture2D*>(&caller);
    auto* rhs_ct2d = this->get_data_slot_.CallAs<compositing::CallTexture2D>();

    if (rhs_ct2d != nullptr) {
        if (!(*rhs_ct2d)(0)) {
            return false;
        } 
    } else {
        core::utility::log::Log::DefaultLog.WriteError("Need a rhs connection.");
        return false;
    }

    if (rhs_ct2d->hasUpdate()) {
        if (show_inspector_.Param<core::param::BoolParam>()->Value()) {
            auto tex_handles = rhs_ct2d->GetInspectorTextures();

            if (tex_handles.size() != 0) {
                ImTextureID tex_handle = &tex_handles[0].first;
                ImVec2 tex_size = ImVec2(tex_handles[0].second.x, tex_handles[0].second.y);

                ImGui::Begin("Simple Texture Inspector");
                ImGuiTexInspect::BeginInspectorPanel("Inspector", tex_handle, tex_size);
                ImGuiTexInspect::EndInspectorPanel();
                ImGui::End();
            }
        }
    }

    lhs_tc->setData(rhs_ct2d->getData(), rhs_ct2d->version());

    return true;
}

bool TextureInspector::getMetaDataCallback(core::Call& caller) {
    return true;
}
