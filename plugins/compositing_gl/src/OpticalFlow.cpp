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
        , uTexture_(nullptr)
        , vTexture_(nullptr)
        , pTexture_(nullptr)
        , deltaUBuffer_(nullptr)
        , isFirstCall_(true)
        , simpleOpticalFlowShader_(nullptr)

        , inputTexSlot_("InputTexture", "Any Texture that should be used to calculate optical flow")
        , flowFieldOutTexSlot("FlowFieldTexture", "Gives access to the resulting flow field texture.")
        , velocityOutTexSlot("VelocityTexture",  "Gives access to the resulting flow field texture.")

        , lambda_("Lambda", "controls trade off between data fidelity term and regularization term")
        , theta_("Theta", "Convex approximation parameter, which affects speed and stability of convergence")
        // , tau_("Tau", "Time step parameter used in fixed-point iteration for updating dual variables")
        // , tolerance_("Tolerance", "Convergence tolerance for change in displacement field")
        // , energyTolerance_("EnergyTolerance", "Convergence energy_tolerance for change in displacement field")
        // , maxIter_("MaxIterations", "Maximum number of iterations")
        // , numLevels_("NumberOfLevels", "Number of levels in image pyramid")  
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


    velocityOutTexSlot.SetCallback(
        CallTexture2D::ClassName(), 
        CallTexture2D::FunctionName(CallTexture2D::CallGetData), 
        &OpticalFlow::getDataCallback
    );
    velocityOutTexSlot.SetCallback(
        CallTexture2D::ClassName(), 
        CallTexture2D::FunctionName(CallTexture2D::CallGetMetaData), 
        &OpticalFlow::getMetaDataCallback
    );
    this->MakeSlotAvailable(&velocityOutTexSlot);

    inputTexSlot_.SetCompatibleCall<CallTexture2DDescription>();
    this->MakeSlotAvailable(&inputTexSlot_);

    // Set up the regularization weight parameter lambda
    lambda_.SetParameter(new core::param::FloatParam(1.f, 0.f, 5.f, 1.f));  // Default: 10, Min: 0.01, Max: 100, Step: 0.1
    this->MakeSlotAvailable(&this->lambda_);
    lambda_.ForceSetDirty();

    // Set up the convex approximation parameter theta
    theta_.SetParameter(new core::param::FloatParam(1.f, 0.f, 5.f, 1.f));  // Default: 10, Min: 0.01, Max: 100, Step: 0.1
    this->MakeSlotAvailable(&this->theta_);
    theta_.ForceSetDirty();

    // // Set up the time step parameter tau
    // tau_.SetParameter(new core::param::FloatParam(0.25f, 0.1f, 0.5f, 0.05f));  // Default: 0.25, Min: 0.1, Max: 0.5, Step: 0.05
    // this->MakeSlotAvailable(&this->tau_);
    // tau_.ForceSetDirty();

    // // Set up the convergence tolerance parameter for change in displacement field u
    // tolerance_.SetParameter(new core::param::FloatParam(1e-4f, 1e-6f, 1e-3f, 1e-1f));  // Default: 1e-4, Min: 1e-6, Max: 1e-3, Step: 1e-5
    // this->MakeSlotAvailable(&this->tolerance_);
    // tolerance_.ForceSetDirty();

    // // Set up the convergence tolerance parameter for change in energy
    // energyTolerance_.SetParameter(new core::param::FloatParam(1e-4f, 1e-6f, 1e-3f, 1e-1f));  // Default: 1e-4, Min: 1e-6, Max: 1e-3, Step: 1e-5
    // this->MakeSlotAvailable(&this->energyTolerance_);
    // energyTolerance_.ForceSetDirty();

    // // Set up the maximum number of iterations parameter
    // maxIter_.SetParameter(new core::param::IntParam(50, 10, 100, 1));  // Default: 100, Min: 10, Max: 500, Step: 10
    // this->MakeSlotAvailable(&this->maxIter_);
    // maxIter_.ForceSetDirty();

    // // Set up the number of pyramid levels parameter
    // numLevels_.SetParameter(new core::param::IntParam(5, 1, 10, 1));  // Default: 4, Min: 1, Max: 6, Step: 1
    // this->MakeSlotAvailable(&this->numLevels_);
    // numLevels_.ForceSetDirty();
}

megamol::compositing_gl::OpticalFlow::~OpticalFlow() {
    this->Release();
}

bool megamol::compositing_gl::OpticalFlow::create() {

    // Create shader options
    auto const shdr_options = core::utility::make_path_shader_options(
        frontend_resources.get<megamol::frontend_resources::RuntimeConfig>());

    try {
        // // Initialize compute shaders
        // updateVShader_ = core::utility::make_glowl_shader(
        //     "update_v", shdr_options, std::filesystem::path("compositing_gl/OpicalFlow/update_v.comp.glsl"));

        // updateUShader_ = core::utility::make_glowl_shader(
        //     "update_u", shdr_options, std::filesystem::path("compositing_gl/OpticalFlow/update_u.comp.glsl"));

        // updatePShader_ = core::utility::make_glowl_shader(
        //     "update_p", shdr_options, std::filesystem::path("compositing_gl/OpticalFlow/update_p.comp.glsl"));

        // computeChangeShader_ = core::utility::make_glowl_shader(
        //     "compute_change", shdr_options, std::filesystem::path("compositing_gl/OpticalFlow/compute_change.comp.glsl"));
        
        simpleOpticalFlowShader_ = core::utility::make_glowl_shader(
            "simple_optical_flow", shdr_options, std::filesystem::path("compositing_gl/OpticalFlow/simple_optical_flow.comp.glsl"));

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

    // // Initialize textures for intermediate variables
    // uTexture_ = std::make_shared<glowl::Texture2D>("u_texture", tex_layout, nullptr);
    vTexture_ = std::make_shared<glowl::Texture2D>("v_texture", tex_layout, nullptr);
    // pTexture_ = std::make_shared<glowl::Texture2D>("p_texture", tex_layout, nullptr);
    // deltaUBuffer_ = std::make_shared<glowl::Texture2D>("delta_u_buffer", tex_layout, nullptr);

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
    bool incomingChange = call_texture != nullptr && call_texture->hasUpdate() ||
                          lambda_.IsDirty() ||
                          theta_.IsDirty(); 
                        //   tau_.IsDirty() ||
                        //   tolerance_.IsDirty() ||
                        //   maxIter_.IsDirty() ||
                        //   numLevels_.IsDirty();

    if (incomingChange) {
        ++version_;


        // Reset dirty flags
        lambda_.ResetDirty();
        theta_.ResetDirty();
        // tau_.ResetDirty();
        // tolerance_.ResetDirty();
        // maxIter_.ResetDirty();
        // numLevels_.ResetDirty();
    
        auto input_tex_2D = call_texture->getData();

        fitTextures(input_tex_2D, {I0_, I1_, outputTex_});

        auto lambdaVal = lambda_.Param<core::param::FloatParam>()->Value();
        auto thetaVal = theta_.Param<core::param::FloatParam>()->Value();
        // auto tauVal = tau_.Param<core::param::FloatParam>()->Value();
        // auto toleranceVal = tolerance_.Param<core::param::FloatParam>()->Value();
        // auto maxIterVal = maxIter_.Param<core::param::IntParam>()->Value();
        // auto numLevels = numLevels_.Param<core::param::IntParam>()->Value();

        if(isFirstCall_) {
            isFirstCall_ = false;
            I0_ = input_tex_2D;
            return true;
        } 

        I1_ = input_tex_2D;

        simpleOpticalFlowShader_->use();
        simpleOpticalFlowShader_->setUniform( "lambda", lambdaVal);
        simpleOpticalFlowShader_->setUniform( "offset", thetaVal);

        bindTextureToShader(simpleOpticalFlowShader_, I0_, "I0", 0);
        bindTextureToShader(simpleOpticalFlowShader_, I1_, "I1", 1);
        outputTex_->bindImage(0, GL_WRITE_ONLY);
        vTexture_->bindImage(1, GL_WRITE_ONLY);

        glDispatchCompute(static_cast<int>(std::ceil(outputTex_->getWidth() / 8.0f)),
            static_cast<int>(std::ceil(outputTex_->getHeight() / 8.0f)), 1);

        glUseProgram(0);

    }

    lhs_tc->setData(outputTex_, version_);
    lhs_tc->setData(vTexture_, version_);

    I0_ = I1_;

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