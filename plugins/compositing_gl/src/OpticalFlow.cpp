/**
 * Luan Bistrovic-Ferizi 
 * Copyright (c) 2024,
 * Karls Eberhardt Universität Tübingen
 */
#include "OpticalFlow.h"

#include <glm/glm.hpp>

#include "compositing_gl/CompositingCalls.h"
#include "mmcore/param/FloatParam.h"
#include "mmcore/param/IntParam.h"
#include "mmcore/param/EnumParam.h"
#include "mmcore/param/BoolParam.h"
#include "mmcore_gl/utility/ShaderFactory.h"

megamol::compositing_gl::OpticalFlow::OpticalFlow()
        : mmstd_gl::ModuleGL()
        , version_(0)
        , outputTex_(nullptr)
        , I0_(nullptr)
        , I1_(nullptr)
        , simpleOpticalFlowShader_(nullptr)
        , passthroughShader_(nullptr)
        , lukasKanadeShader_(nullptr)
        , isFirstCall_(true)

        , inputTexSlot_("InputTexture", "Any Texture that should be used to calculate optical flow")
        , flowFieldOutTexSlot("FlowFieldTexture", "Gives access to the resulting flow field texture.")

        , offset_("Offset", "Convex approximation parameter, which affects speed and stability of convergence")
        , frameRateAdjust_("Framerate-Adjust", "Adjustment of the megamol framework pipeline velocity")
        , windowSize_("WindowSize", "Size of the window for Lukas Kanade Method")
{

    flowFieldOutTexSlot.SetCallback(
        CallTexture2D::ClassName(), 
        CallTexture2D::FunctionName(CallTexture2D::CallGetData), 
        &OpticalFlow::getDataCallback
    );
    flowFieldOutTexSlot.SetCallback(
        CallTexture2D::ClassName(), 
        CallTexture2D::FunctionName(CallTexture2D::CallGetMetaData), 
        &OpticalFlow::getMetaDataCallback
    );
    this->MakeSlotAvailable(&flowFieldOutTexSlot);

    inputTexSlot_.SetCompatibleCall<CallTexture2DDescription>();
    this->MakeSlotAvailable(&inputTexSlot_);

    // Set up the convex approximation parameter theta
    offset_.SetParameter(new core::param::IntParam(1, 1, 5, 1));  // Default: 10, Min: 0.01, Max: 100, Step: 0.1
    this->MakeSlotAvailable(&this->offset_);
    offset_.ForceSetDirty();

    frameRateAdjust_.SetParameter(new core::param::IntParam(10, 0, 50, 1));  // Default: 10, Min: 0.01, Max: 100, Step: 0.1
    this->MakeSlotAvailable(&this->frameRateAdjust_);
    frameRateAdjust_.ForceSetDirty();

    windowSize_.SetParameter(new core::param::IntParam(1, 1, 10, 1));  // Default: 10, Min: 0.01, Max: 100, Step: 0.1
    this->MakeSlotAvailable(&this->windowSize_);
    windowSize_.ForceSetDirty();
}

megamol::compositing_gl::OpticalFlow::~OpticalFlow() {
    this->Release();
}

bool megamol::compositing_gl::OpticalFlow::create() {

    // Create shader options
    auto const shdr_options = core::utility::make_path_shader_options(
        frontend_resources.get<megamol::frontend_resources::RuntimeConfig>());

    try {
        passthroughShader_ = core::utility::make_glowl_shader(
            "passthrough", shdr_options, std::filesystem::path("compositing_gl/passthrough.comp.glsl"));
        
        simpleOpticalFlowShader_ = core::utility::make_glowl_shader(
            "simple_optical_flow", shdr_options, std::filesystem::path("compositing_gl/OpticalFlow/simple_optical_flow.comp.glsl"));

        lukasKanadeShader_ = core::utility::make_glowl_shader(
            "simple_optical_flow", shdr_options, std::filesystem::path("compositing_gl/OpticalFlow/lukas_kanade_method.comp.glsl"));

    } catch (glowl::GLSLProgramException const& ex) {
        megamol::core::utility::log::Log::DefaultLog.WriteError("[TVL1OpticalFlow] Shader compilation error: %s", ex.what());
        return false;
    } catch (std::exception const& ex) {
        megamol::core::utility::log::Log::DefaultLog.WriteError(
            "[OpticalFlow] Unable to compile shader: Unknown exception: %s", ex.what());
        return false;
    } catch (...) {
        megamol::core::utility::log::Log::DefaultLog.WriteError(
            "[OpticalFlow] Unable to compile shader: Unknown exception.");
        return false;
    }

    // Create textures with initial layout
    glowl::TextureLayout tex_layout{GL_RGBA16F, 1, 1, 1, GL_RGBA, GL_FLOAT, 1}; // Use appropriate format and type for your data

    // Initialize input textures
    I0_ = std::make_shared<glowl::Texture2D>("I0", tex_layout, nullptr);
    I1_ = std::make_shared<glowl::Texture2D>("I1", tex_layout, nullptr);

    // Initialize the final output texture
    outputTex_ = std::make_shared<glowl::Texture2D>("output_texture", tex_layout, nullptr);

    return true;
}


void megamol::compositing_gl::OpticalFlow::release() {}

bool megamol::compositing_gl::OpticalFlow::getDataCallback(core::Call& caller) {
    auto lhs_tc = dynamic_cast<CallTexture2D*>(&caller);
    auto call_texture = inputTexSlot_.CallAs<CallTexture2D>();

    if (lhs_tc == nullptr) {
        return false;
    }

    if(call_texture != nullptr) {
        if(!(*call_texture)(0)){
            return false;
        }
    }

    // Check if any incoming data has changed
    bool incomingChange = call_texture != nullptr && call_texture->hasUpdate() || offset_.IsDirty(); 

    if(frameRateAdjust_.IsDirty()) {
        counter = frameRateAdjust_.Param<core::param::IntParam>()->Value();
        frameRateAdjust_.ResetDirty();
    }

    if (incomingChange) {
        ++version_;

        auto input_tex_2D = call_texture->getData();
        fitTextures(input_tex_2D, {I0_, I1_, outputTex_});

        offset_.ResetDirty();

        auto offsetVal = offset_.Param<core::param::IntParam>()->Value();
        auto frameRateVal = frameRateAdjust_.Param<core::param::IntParam>()->Value();
        auto windowSizeVal = windowSize_.Param<core::param::IntParam>()->Value();

        if(isFirstCall_) {
            isFirstCall_ = false;
            textureCopy(input_tex_2D, I0_); 
            textureCopy(input_tex_2D, outputTex_);
            return true;
        } 

        if(counter == 0) {
            textureCopy(input_tex_2D, I1_); 

            lukasKanadeShader_->use();
            lukasKanadeShader_->setUniform( "offset", offsetVal);
            lukasKanadeShader_->setUniform( "windowSize", windowSizeVal);

            bindTextureToShader(lukasKanadeShader_, I0_, "prev_frame", 0);
            bindTextureToShader(lukasKanadeShader_, I1_, "next_frame", 1);
            outputTex_->bindImage(0, GL_WRITE_ONLY);

            glDispatchCompute(static_cast<int>(std::ceil(outputTex_->getWidth() / 8.0f)),
                static_cast<int>(std::ceil(outputTex_->getHeight() / 8.0f)), 1);
            glUseProgram(0);

            textureCopy(I1_, I0_);
            counter = frameRateVal;

            // simpleOpticalFlowShader_->use();
            // simpleOpticalFlowShader_->setUniform( "offset", offsetVal);

            // bindTextureToShader(simpleOpticalFlowShader_, I0_, "I0", 0);
            // bindTextureToShader(simpleOpticalFlowShader_, I1_, "I1", 1);
            // outputTex_->bindImage(0, GL_WRITE_ONLY);

            // glDispatchCompute(static_cast<int>(std::ceil(outputTex_->getWidth() / 8.0f)),
            //     static_cast<int>(std::ceil(outputTex_->getHeight() / 8.0f)), 1);
            // glUseProgram(0);

            // textureCopy(I1_, I0_);
            // counter = frameRateVal;
        }
    }
    counter--;

    lhs_tc->setData(outputTex_, version_);

    return true;
}

bool megamol::compositing_gl::OpticalFlow::getMetaDataCallback(core::Call& caller) {
    return true;
}

void megamol::compositing_gl::OpticalFlow::fitTextures(
    std::shared_ptr<glowl::Texture2D> source, 
    std::vector<std::shared_ptr<glowl::Texture2D>> goalTextures) 
{
    std::pair<int, int> resolution(source->getWidth(), source->getHeight());
    for (auto& tex : goalTextures) {
        if (tex->getWidth() != resolution.first || tex->getHeight() != resolution.second) {
            glowl::TextureLayout tx_layout{
                GL_RGBA16F, resolution.first, resolution.second, 1, GL_RGBA, GL_HALF_FLOAT, 1};
            tex->reload(tx_layout, nullptr);
        }
    }
}

void megamol::compositing_gl::OpticalFlow::bindTextureToShader(
    std::unique_ptr<glowl::GLSLProgram>& shader,
    std::shared_ptr<glowl::Texture2D> texture, 
    const char* tex_name,
    int num 
) {
    glActiveTexture(GL_TEXTURE0 + num);
    texture->bindTexture();
    glUniform1i(shader->getUniformLocation(tex_name), num);
}

void megamol::compositing_gl::OpticalFlow::textureCopy(
    std::shared_ptr<glowl::Texture2D> inputTex, 
    std::shared_ptr<glowl::Texture2D> outputTex
) {
    passthroughShader_->use();
    glActiveTexture(GL_TEXTURE0);
    inputTex->bindTexture();
    glUniform1i(passthroughShader_->getUniformLocation("input_tex"), 0);
    outputTex->bindImage(0, GL_WRITE_ONLY);

    glDispatchCompute(static_cast<int>(std::ceil(outputTex_->getWidth() / 8.0f)),
            static_cast<int>(std::ceil(outputTex_->getHeight() / 8.0f)), 1);

    glUseProgram(0);
}