/**
 * Luan Bistrovic-Ferizi 
 * Copyright (c) 2024,
 * Karls Eberhardt Universität Tübingen
 */
#include "MotionBlur.h"

#include <glm/glm.hpp>

#include "compositing_gl/CompositingCalls.h"
#include "mmcore/param/FloatParam.h"
#include "mmcore/param/IntParam.h"
#include "mmcore/param/EnumParam.h"
#include "mmcore/param/BoolParam.h"
#include "mmcore_gl/utility/ShaderFactory.h"

megamol::compositing_gl::MotionBlur::MotionBlur()
        : mmstd_gl::ModuleGL()
        , version_(0)
        , outputTex_(nullptr)
        , blurShaderProgram_(nullptr)
        , tileMaxShaderProgram_(nullptr)
        , neighborMaxShaderProgram_(nullptr)
        , tileMaxBuffer_(nullptr)
        , neighborMaxBuffer_(nullptr)
        , velocityShaderProgram_(nullptr)
        , upscaleShader_(nullptr)

        , outputTexSlot_("OutputTexture", "Gives access to the resulting output texture.")
        , inputColorSlot_("ColorTexture", "Connects the color texture.")
        , inputFlowSlot_("FlowTexture", "Connects Flow Field Buffer")
        , inputDepthSlot_("DepthTexture", "Connects Depht Texture") 

        , maxBlurRadius_("MaxBlurRadius", "Maximal Blur Radius") 
        , sampleTaps_("NumSamples", "Number of Samples to be taken")
        , exposureTime_("ExposureTime", "Exposure Time")
        , frameRate_("FrameRate", "Framerate of the animation")
{

    outputTexSlot_.SetCallback(
        CallTexture2D::ClassName(), 
        CallTexture2D::FunctionName(CallTexture2D::CallGetData), 
        &MotionBlur::getDataCallback
    );
    outputTexSlot_.SetCallback(
        CallTexture2D::ClassName(), 
        CallTexture2D::FunctionName(CallTexture2D::CallGetMetaData), 
        &MotionBlur::getMetaDataCallback
    );
    this->MakeSlotAvailable(&outputTexSlot_);

    inputColorSlot_.SetCompatibleCall<CallTexture2DDescription>();
    this->MakeSlotAvailable(&inputColorSlot_);

    inputDepthSlot_.SetCompatibleCall<CallTexture2DDescription>();
    this->MakeSlotAvailable(&inputDepthSlot_);

    inputFlowSlot_.SetCompatibleCall<CallTexture2DDescription>();
    this->MakeSlotAvailable(&inputFlowSlot_);

    maxBlurRadius_.SetParameter(new core::param::IntParam(3, 1, 100, 1));
    this->MakeSlotAvailable(&this->maxBlurRadius_);
    maxBlurRadius_.ForceSetDirty();

    sampleTaps_.SetParameter(new core::param::IntParam(5, 0, 20, 1));
    this->MakeSlotAvailable(&this->sampleTaps_);
    sampleTaps_.ForceSetDirty();

    exposureTime_.SetParameter(new core::param::FloatParam(0.01, 0, 1, 0.001));
    this->MakeSlotAvailable(&this->exposureTime_);
    exposureTime_.ForceSetDirty();

    frameRate_.SetParameter(new core::param::IntParam(60, 0 , 500, 1));
    this->MakeSlotAvailable(&this->frameRate_);
    frameRate_.ForceSetDirty();
}


megamol::compositing_gl::MotionBlur::~MotionBlur() {
    this->Release();
}

bool megamol::compositing_gl::MotionBlur::create() {

    auto const shdr_options =
        core::utility::make_path_shader_options(frontend_resources.get<megamol::frontend_resources::RuntimeConfig>());

    try {
        blurShaderProgram_ = core::utility::make_glowl_shader(
            "MotionBlur", shdr_options, std::filesystem::path("compositing_gl/MotionBlur/motion_blur.comp.glsl"));

        velocityShaderProgram_ = core::utility::make_glowl_shader(
            "VelocityCalc", shdr_options, std::filesystem::path("compositing_gl/MotionBlur/velocity_calc.comp.glsl"));

        tileMaxShaderProgram_ = core::utility::make_glowl_shader(
            "TileMax", shdr_options, std::filesystem::path("compositing_gl/MotionBlur/tile_max_calc.comp.glsl"));

        neighborMaxShaderProgram_ = core::utility::make_glowl_shader(
            "NeighborMax", shdr_options, std::filesystem::path("compositing_gl/MotionBlur/neighbor_max_calc.comp.glsl"));

        upscaleShader_ = core::utility::make_glowl_shader(
            "Upscale", shdr_options, std::filesystem::path("compositing_gl/MotionBlur/upscale.comp.glsl"));

    } catch (glowl::GLSLProgramException const& ex) {
        megamol::core::utility::log::Log::DefaultLog.WriteError("[Contours] %s", ex.what());
    } catch (std::exception const& ex) {
        megamol::core::utility::log::Log::DefaultLog.WriteError(
            "[Contours] Unable to compile shader: Unknown exception: %s", ex.what());
    } catch (...) {
        megamol::core::utility::log::Log::DefaultLog.WriteError(
            "[Contours] Unable to compile shader: Unknown exception.");
    }

    glowl::TextureLayout tx_layout{GL_RGBA16F, 1, 1, 1, GL_RGBA, GL_HALF_FLOAT, 1};

    outputTex_ = std::make_shared<glowl::Texture2D>("filtered_output", tx_layout, nullptr);
    velocityBuffer_ = std::make_shared<glowl::Texture2D>("velocity_buffer", tx_layout, nullptr);
    tileMaxBuffer_ = std::make_shared<glowl::Texture2D>("tile_max_buffer", tx_layout, nullptr);
    neighborMaxBuffer_ = std::make_shared<glowl::Texture2D>("neighbor_max_buffer", tx_layout, nullptr);

    return true;
}

void megamol::compositing_gl::MotionBlur::release() {}

bool megamol::compositing_gl::MotionBlur::getDataCallback(core::Call& caller) {
    auto lhs_tc = dynamic_cast<CallTexture2D*>(&caller);
    auto call_color  = inputColorSlot_.CallAs<CallTexture2D>();
    auto call_depth = inputDepthSlot_.CallAs<CallTexture2D>();
    auto call_flow = inputFlowSlot_.CallAs<CallTexture2D>();

    if (lhs_tc == nullptr) {
        return false;
    }

    if(call_color != nullptr) {
        if(!(*call_color)(0)) {
            return false;
        }
    }

    if(call_depth != nullptr) {
        if(!(*call_depth)(0)) {
            return false;
        }
    }

    if(call_flow != nullptr) {
        if(!(*call_flow)(0)) {
            return false;
        }
    }


    bool incomingChange = call_color != nullptr && call_color->hasUpdate() ||
                        call_depth != nullptr && call_depth->hasUpdate() ||
                        call_flow != nullptr && call_flow->hasUpdate() ||
                        maxBlurRadius_.IsDirty() || sampleTaps_.IsDirty() || 
                        exposureTime_.IsDirty() || frameRate_.IsDirty();

    if (incomingChange) {
        ++version_;

        maxBlurRadius_.ResetDirty();
        sampleTaps_.ResetDirty();
        exposureTime_.ResetDirty();
        frameRate_.ResetDirty();

        auto color_tex_2D = call_color->getData();
        auto depth_tex_2D = call_depth->getData();
        auto flow_tex_2D = call_flow->getData();

        auto maxBlurRadiusVal = maxBlurRadius_.Param<core::param::IntParam>()->Value();
        auto sampleTapsVal = sampleTaps_.Param<core::param::IntParam>()->Value();
        auto frameRateVal = frameRate_.Param<core::param::IntParam>()->Value();
        auto exposureTimeVal = exposureTime_.Param<core::param::FloatParam>()->Value();
        int tileSize = maxBlurRadiusVal * 2 + 1;


        fitTextures(color_tex_2D, {velocityBuffer_, outputTex_, tileMaxBuffer_, neighborMaxBuffer_});

        if(velocityShaderProgram_ != nullptr) {
            velocityShaderProgram_->use();
            velocityShaderProgram_->setUniform("exposureTime", exposureTimeVal);
            velocityShaderProgram_->setUniform("frameRate", frameRateVal);
            velocityShaderProgram_->setUniform("maxBlurRadius", maxBlurRadiusVal);
            bindTextureToShader(velocityShaderProgram_, flow_tex_2D, "flow_tex_2D", 0);

            velocityBuffer_->bindImage(0, GL_WRITE_ONLY);

            glDispatchCompute(static_cast<int>(std::ceil(outputTex_->getWidth() / 8.0f)),
                            static_cast<int>(std::ceil(outputTex_->getHeight() / 8.0f)), 1);

            glUseProgram(0);

            glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
        }

        if(tileMaxShaderProgram_ != nullptr) {
            tileMaxShaderProgram_->use();
            tileMaxShaderProgram_->setUniform("maxBlurRadius", maxBlurRadiusVal);
            bindTextureToShader(tileMaxShaderProgram_, velocityBuffer_, "velocityBuffer", 0);

            tileMaxBuffer_->bindImage(0, GL_WRITE_ONLY);

            glDispatchCompute(static_cast<int>(std::ceil(tileMaxBuffer_->getWidth() / 8.0)),
                            static_cast<int>(std::ceil(tileMaxBuffer_->getHeight())), 1);


            glUseProgram(0);

            glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
        }

        if(neighborMaxShaderProgram_ != nullptr) {
            neighborMaxShaderProgram_->use();
            neighborMaxShaderProgram_->setUniform("maxBlurRadius", maxBlurRadiusVal);
            bindTextureToShader(neighborMaxShaderProgram_, tileMaxBuffer_, "tileMaxBuffer", 0);

            outputTex_->bindImage(0, GL_WRITE_ONLY);

            glDispatchCompute(static_cast<int>(std::ceil(neighborMaxBuffer_->getWidth() / 8.0)),
                            static_cast<int>(std::ceil(neighborMaxBuffer_->getHeight() / 8.0)), 1);

            glUseProgram(0);

            glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
        } 

        if(blurShaderProgram_ != nullptr) {
            blurShaderProgram_->use();
            blurShaderProgram_->setUniform("maxBlurRadius", maxBlurRadiusVal);
            blurShaderProgram_->setUniform("numSamples", sampleTapsVal);
            blurShaderProgram_->setUniform("exposureTime", exposureTimeVal);
            blurShaderProgram_->setUniform("frameRate", frameRateVal);

            bindTextureToShader(blurShaderProgram_, color_tex_2D, "colorBuffer", 0);
            bindTextureToShader(blurShaderProgram_, depth_tex_2D, "depthBuffer", 1);
            bindTextureToShader(blurShaderProgram_, neighborMaxBuffer_, "neighborMaxBuffer", 2);

            outputTex_->bindImage(0, GL_WRITE_ONLY);

            glDispatchCompute(static_cast<int>(std::ceil(color_tex_2D->getWidth() / 8.0f)),
                            static_cast<int>(std::ceil(color_tex_2D->getHeight() / 8.0f)), 1);

            glUseProgram(0);
        }
    }

    lhs_tc->setData(outputTex_, version_);

    return true;
}

bool megamol::compositing_gl::MotionBlur::getMetaDataCallback(core::Call& caller) {
    return true;
}

void megamol::compositing_gl::MotionBlur::fitTextures(
    std::shared_ptr<glowl::Texture2D> source, 
    std::vector<std::shared_ptr<glowl::Texture2D>> goalTextures) 
{
    if (!source) return; 
    int width = source->getWidth();
    int height = source->getHeight();

    for (auto& tex : goalTextures) {
        if (tex && (tex->getWidth() != width || tex->getHeight() != height)) {
            glowl::TextureLayout layout{
                GL_RGBA16F, width, height, 1, GL_RGBA, GL_HALF_FLOAT, 1};
            tex->reload(layout, nullptr);
        }
    }
}


void megamol::compositing_gl::MotionBlur::bindTextureToShader(
    std::unique_ptr<glowl::GLSLProgram>& shader,
    std::shared_ptr<glowl::Texture2D> texture, 
    const char* tex_name,
    int num 
) {
    glActiveTexture(GL_TEXTURE0 + num);
    texture->bindTexture();
    glUniform1i(shader->getUniformLocation(tex_name), num);
}