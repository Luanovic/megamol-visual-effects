
/**
 * Luan Bistrovic-Ferizi 
 * Copyright (c) 2024,
 * Karls Eberhardt Universität Tübingen
 */
#include "MedianFilter.h"

#include <glm/glm.hpp>

#include "compositing_gl/CompositingCalls.h"
#include "mmcore/param/FloatParam.h"
#include "mmcore/param/IntParam.h"
#include "mmcore/param/EnumParam.h"
#include "mmcore/param/BoolParam.h"
#include "mmcore_gl/utility/ShaderFactory.h"

megamol::compositing_gl::MedianFilter::MedianFilter()
        : mmstd_gl::ModuleGL()
        , version_(0)
        , outputTex_(nullptr)

        , outputTexSlot_("OutputTexture", "Gives access to the resulting output texture.")
        , inputColorSlot_("ColorTexture", "Connects the color texture.")
        , medianFilterProgram_(nullptr)
        // , beta("Threshold", "Threshold for Median Filter Algorithm")
        // , windowSize("WindowSize", "Window size N of the filter with dimenstion NxN")
{

    outputTexSlot_.SetCallback(
        CallTexture2D::ClassName(), 
        CallTexture2D::FunctionName(CallTexture2D::CallGetData), 
        &MedianFilter::getDataCallback
    );
    outputTexSlot_.SetCallback(
        CallTexture2D::ClassName(), 
        CallTexture2D::FunctionName(CallTexture2D::CallGetMetaData), 
        &MedianFilter::getMetaDataCallback
    );
    this->MakeSlotAvailable(&outputTexSlot_);

    // inputColorSlot_.SetCompatibleCall<CallTexture2DDescription>();
    // this->MakeSlotAvailable(&inputColorSlot_);

    // beta.SetParameter(new core::param::FloatParam(0.5f, 0.0f, 10.f, 0.1f));
    // this->makeSlotAvailable(&this->beta);


}

megamol::compositing_gl::MedianFilter::~MedianFilter() {
    this->Release();
}

bool megamol::compositing_gl::MedianFilter::create() {

    auto const shdr_options =
        core::utility::make_path_shader_options(frontend_resources.get<megamol::frontend_resources::RuntimeConfig>());

    try {
        medianFilterProgram_ = core::utility::make_glowl_shader(
            "MedianFilter", shdr_options, std::filesystem::path("compositing_gl/median_filter.comp.glsl"));

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
    outputTex_ = std::make_shared<glowl::Texture2D>("contours_output", tx_layout, nullptr);

    return true;
}

void megamol::compositing_gl::MedianFilter::release() {}

bool megamol::compositing_gl::MedianFilter::getDataCallback(core::Call& caller) {
    auto lhs_tc = dynamic_cast<CallTexture2D*>(&caller);
    auto call_color  = inputColorSlot_.CallAs<CallTexture2D>();

    if (lhs_tc == nullptr) {
        return false;
    }

    if(call_color != nullptr) {
        if(!(*call_color)(0)) {
            return false;
        }
    }

    bool incomingChange = call_color != nullptr && call_color->hasUpdate();

    if (incomingChange) {
        ++version_;

        auto color_tex_2D = call_color->getData();

        fitTextures(color_tex_2D);

        if(medianFilterProgram_ != nullptr) {
            medianFilterProgram_->use(); 
            this->bindTexture(medianFilterProgram_, color_tex_2D, "color_tex_2D", 0);
            outputTex_->bindImage(0, GL_WRITE_ONLY);

            glDispatchCompute(static_cast<int>(std::ceil(outputTex_->getWidth() / 8.0f)),
                static_cast<int>(std::ceil(outputTex_->getHeight() / 8.0f)), 1);

            glUseProgram(0);
        }
    }

    lhs_tc->setData(outputTex_, version_);

    return true;
}

bool megamol::compositing_gl::MedianFilter::getMetaDataCallback(core::Call& caller) {
    return true;
}

void megamol::compositing_gl::MedianFilter::fitTextures(std::shared_ptr<glowl::Texture2D> source) {
    std::pair<int, int> resolution(source->getWidth(), source->getHeight());
    std::vector<std::shared_ptr<glowl::Texture2D>> texVec = {outputTex_};
    for (auto& tex : texVec) {
        if (tex->getWidth() != resolution.first || tex->getHeight() != resolution.second) {
            glowl::TextureLayout tx_layout{
                GL_RGBA16F, resolution.first, resolution.second, 1, GL_RGBA, GL_HALF_FLOAT, 1};
            tex->reload(tx_layout, nullptr);
        }
    }
}

void megamol::compositing_gl::MedianFilter::bindTexture(
    std::unique_ptr<glowl::GLSLProgram>& shader,
    std::shared_ptr<glowl::Texture2D> texture, 
    const char* tex_name,
    int num 
) {
    std::vector<int> glTex = {GL_TEXTURE0, GL_TEXTURE1, GL_TEXTURE2, GL_TEXTURE3, GL_TEXTURE4 };
    glActiveTexture(glTex[num]);
    texture->bindTexture();
    glUniform1i(shader->getUniformLocation(tex_name), num);
}