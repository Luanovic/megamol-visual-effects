/*
 * DepthPeelingRenderTarget.h
 *
 * Copyright (C) 2019 by Universitaet Stuttgart (VISUS).
 * All rights reserved.
 */

#pragma once


#include <glowl/FramebufferObject.hpp>

#include "CompositingOutHandler.h"
#include "mmcore/CalleeSlot.h"
#include "mmstd/renderer/RendererModule.h"
#include "mmstd_gl/ModuleGL.h"
#include "mmstd_gl/renderer/CallRender3DGL.h"

namespace megamol::compositing_gl {

class MotionBlur : public core::view::RendererModule<mmstd_gl::CallRenderer3DGL, mmstd_gl::ModuleGL> {
public: 
    static const char* ClassName() {
        return "MotionBlur";
    }

    static const char* Description() {
        return "Creates a Plugin for the Postprocessing Effect of Motion Blur";
    }

    static bool IsAvailable() {
        return true;
    }

    MotionBlur();
    ~MotionBlur() override;

protected:
    /**
     * Implementation of 'Create'.
     *
     * @return 'true' on success, 'false' otherwise.
     */
    bool create() override;

    /**
     * Implementation of 'Release'.
     */
    void release() override;

    /**
     * The get extents callback. The module should set the members of
     * 'call' to tell the caller the extents of its data (bounding boxes
     * and times).
     *
     * @param call The calling call.
     *
     * @return The return value of the function.
     */
    bool GetExtents(mmstd_gl::CallRender3DGL& call) override;

    /**
     * The render callback.
     *
     * @param call The calling call.
     *
     * @return The return value of the function.
     */
    bool Render(mmstd_gl::CallRender3DGL& call) override;

    /**
     *
     */
    bool getColorRenderTarget(core::Call& caller);

    /**
     *
     */
    bool getDepthBuffer(core::Call& caller);

    /**
     *
     */
    bool getCameraSnapshot(core::Call& caller);

    /**
     *
     */
    bool getFramebufferObject(core::Call& caller);

    /**
     *
     */
    bool getMetaDataCallback(core::Call& caller);

    /**
     * \brief Sets texture format variables.
     *
     *  @return 'true' if updates sucessfull, 'false' otherwise
     */
    bool textureFormatUpdate();

    /**
     * G-Buffer for deferred rendering. By default if uses three color attachments (and a depth renderbuffer):
     * surface albedo - RGB 16bit per channel
     * normals - RGB 16bit per channel
     * depth - single channel 32bit
     */
    std::shared_ptr<glowl::FramebufferObject> m_GBuffer;

    uint32_t m_version;

private:
    /** Local copy of last used camera*/
    core::view::Camera m_last_used_camera;

     core::param::ParamSlot layers;

    core::CalleeSlot m_color_render_target;
    core::CalleeSlot m_normal_render_target;
    core::CalleeSlot m_depth_buffer;

    /** Slot for accessing the camera that is propagated to the render chain from this module */
    core::CalleeSlot m_camera;

    /** Slot for accessing the framebuffer object used by this render target module */
    core::CalleeSlot m_framebuffer_slot;

    CompositingOutHandler colorOutHandler_;
    CompositingOutHandler normalsOutHandler_;
};

}