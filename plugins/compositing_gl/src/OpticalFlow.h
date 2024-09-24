

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

class OpticalFlow : public mmstd_gl::ModuleGL {
public:
    /**
     * Answer the name of this module.
     *
     * @return The name of this module.
     */
    static const char* ClassName() {
        return "OpticalFlow";
    }

    /**
     * Answer a human readable description of this module.
     *
     * @return A human readable description of this module.
     */
    static const char* Description() {
        return "Compositing module that creates the optical Flow field of scenes";
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
    OpticalFlow();

    /** Dtor. */
    ~OpticalFlow() override;

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

    void textureCopy(
        std::shared_ptr<glowl::Texture2D> inputTex, 
        std::shared_ptr<glowl::Texture2D> outputTex 
    );

    bool isFirstCall_;
    int counter;

    /** Slot for the output texture */
    core::CalleeSlot flowFieldOutTexSlot;

    /** Slot receiving the input color texture */
    core::CallerSlot inputTexSlot_;

    core::param::ParamSlot offset_;
    core::param::ParamSlot frameRateAdjust_;

    /** version identifier */
    uint32_t version_;

    /** shader performing the conotur calculations */
    std::unique_ptr<glowl::GLSLProgram> simpleOpticalFlowShader_;
    std::unique_ptr<glowl::GLSLProgram> passthroughShader_;

    std::shared_ptr<glowl::Texture2D> I0_;
    std::shared_ptr<glowl::Texture2D> I1_;
    std::shared_ptr<glowl::Texture2D> outputTex_;
    };
} 
