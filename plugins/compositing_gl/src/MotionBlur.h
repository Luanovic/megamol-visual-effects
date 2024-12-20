/**
 * MegaMol
 * Copyright (c) 2023, MegaMol Dev Team
 * All rights reserved.
 */
#pragma once

#include <glowl/BufferObject.hpp>
#include <glowl/GLSLProgram.hpp>
#include <glowl/Texture2D.hpp>

#include "mmcore/CalleeSlot.h"
#include "mmcore/CallerSlot.h"
#include "mmcore/param/ParamSlot.h"

#include "mmstd_gl/ModuleGL.h"

namespace megamol::compositing_gl {

class MotionBlur : public mmstd_gl::ModuleGL {
public:
    /**
     * Answer the name of this module.
     *
     * @return The name of this module.
     */
    static const char* ClassName() {
        return "MotionBlur";
    }

    /**
     * Answer a human readable description of this module.
     *
     * @return A human readable description of this module.
     */
    static const char* Description() {
        return "Module implementin Motion Blur";
    }

    /**
     * Answers whether this module is available on the current system.
     *
     * @return 'true' if the module is available, 'false' otherwise.
     */
    static bool IsAvailable() {
        return true;
    }

    /** Ctor. */
    MotionBlur();

    /** Dtor. */
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
     * Implementation of 'getData'.
     *
     * @param caller The calling call
     * @return 'true' on success, 'false' otherwise.
     */
    bool getDataCallback(core::Call& caller);

    /**
     * Implementation of 'getMetaData'.
     *
     * @param caller The calling call
     * @return 'true' on success, 'false' otherwise.
     */
    bool getMetaDataCallback(core::Call& caller);

private:
    /**
     * Fits all internal textures of this module to the size of the given one
     *
     * @param source The texture taken as template for the local ones
     */
    void fitTextures(
        std::shared_ptr<glowl::Texture2D> source,
        std::vector<std::shared_ptr<glowl::Texture2D>> goalTextures
    );


    void bindTextureToShader(
        std::unique_ptr<glowl::GLSLProgram>& shader,
        std::shared_ptr<glowl::Texture2D> texture, 
        const char* tex_name, 
        int num
    );

    /** Slot for the output texture */
    core::CalleeSlot outputTexSlot_;

    /** Slot receiving the input color texture */
    core::CallerSlot inputColorSlot_;
    core::CallerSlot inputFlowSlot_;
    core::CallerSlot inputDepthSlot_;

    core::param::ParamSlot maxBlurRadius_;
    core::param::ParamSlot sampleTaps_;
    core::param::ParamSlot exposureTime_;
    core::param::ParamSlot frameRate_;
    core::param::ParamSlot zExtendScalar_;

    /** version identifier */
    uint32_t version_;

    /** shader performing the conotur calculations */
    std::unique_ptr<glowl::GLSLProgram> blurShaderProgram_;
    std::unique_ptr<glowl::GLSLProgram> velocityShaderProgram_;
    std::unique_ptr<glowl::GLSLProgram> tileMaxShaderProgram_;
    std::unique_ptr<glowl::GLSLProgram> neighborMaxShaderProgram_;
    std::unique_ptr<glowl::GLSLProgram> upscaleShader_;


    std::shared_ptr<glowl::Texture2D> velocityBuffer_;

    /** holds max velocity for each tile (neighborhood of pixels) */
    std::shared_ptr<glowl::Texture2D> tileMaxBuffer_;

    /** */
    std::shared_ptr<glowl::Texture2D> neighborMaxBuffer_;

    /** final output texture */
    std::shared_ptr<glowl::Texture2D> outputTex_;


    glm::ivec2 m_last_tex_size = glm::ivec2(0, 0);
    };
} 


