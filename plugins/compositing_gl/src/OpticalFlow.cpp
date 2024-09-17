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

        , inputTexSlot_("InputTexture", "Any Texture that should be used to calculate optical flow")
        , outputTexSlot_("OutputTexture", "Gives access to the resulting output texture.")

        , lambda_("Lambda", "controls trade off between data fidelity term and regularization term")
        , theta_("Theta", "Convex approximation parameter, which affects speed and stability of convergence")
        , tau_("Tau", "Time step parameter used in fixed-point iteration for updating dual variables")
        , tolerance_("Tolerance", "Convergence tolerance for change in displacement field")
        , energyTolerance_("EnergyTolerance", "Convergence energy_tolerance for change in displacement field")
        , maxIter_("MaxIterations", "Maximum number of iterations")
        , numLevels_("NumberOfLevels", "Number of levels in image pyramid")  
{

    outputTexSlot_.SetCallback(
        CallTexture2D::ClassName(), 
        CallTexture2D::FunctionName(CallTexture2D::CallGetData), 
        &OpticalFlow::getDataCallback
    );
    outputTexSlot_.SetCallback(
        CallTexture2D::ClassName(), 
        CallTexture2D::FunctionName(CallTexture2D::CallGetMetaData), 
        &OpticalFlow::getMetaDataCallback
    );
    this->MakeSlotAvailable(&outputTexSlot_);

    inputTexSlot_.SetCompatibleCall<CallTexture2DDescription>();
    this->MakeSlotAvailable(&inputTexSlot_);

    // Set up the regularization weight parameter lambda
    lambda_.SetParameter(new core::param::FloatParam(10.0f, 0.01f, 100.0f, 0.1f));  // Default: 10, Min: 0.01, Max: 100, Step: 0.1
    this->MakeSlotAvailable(&this->lambda_);
    lambda_.ForceSetDirty();

    // Set up the convex approximation parameter theta
    theta_.SetParameter(new core::param::FloatParam(0.1f, 0.01f, 0.5f, 0.01f));  // Default: 0.1, Min: 0.01, Max: 0.5, Step: 0.01
    this->MakeSlotAvailable(&this->theta_);
    theta_.ForceSetDirty();

    // Set up the time step parameter tau
    tau_.SetParameter(new core::param::FloatParam(0.25f, 0.1f, 0.5f, 0.05f));  // Default: 0.25, Min: 0.1, Max: 0.5, Step: 0.05
    this->MakeSlotAvailable(&this->tau_);
    tau_.ForceSetDirty();

    // Set up the convergence tolerance parameter for change in displacement field u
    tolerance_.SetParameter(new core::param::FloatParam(1e-4f, 1e-6f, 1e-3f, 1e-1f));  // Default: 1e-4, Min: 1e-6, Max: 1e-3, Step: 1e-5
    this->MakeSlotAvailable(&this->tolerance_);
    tolerance_.ForceSetDirty();

    // Set up the convergence tolerance parameter for change in energy
    energyTolerance_.SetParameter(new core::param::FloatParam(1e-4f, 1e-6f, 1e-3f, 1e-1f));  // Default: 1e-4, Min: 1e-6, Max: 1e-3, Step: 1e-5
    this->MakeSlotAvailable(&this->energyTolerance_);
    energyTolerance_.ForceSetDirty();

    // Set up the maximum number of iterations parameter
    maxIter_.SetParameter(new core::param::IntParam(50, 10, 100, 1));  // Default: 100, Min: 10, Max: 500, Step: 10
    this->MakeSlotAvailable(&this->maxIter_);
    maxIter_.ForceSetDirty();

    // Set up the number of pyramid levels parameter
    numLevels_.SetParameter(new core::param::IntParam(5, 1, 10, 1));  // Default: 4, Min: 1, Max: 6, Step: 1
    this->MakeSlotAvailable(&this->numLevels_);
    numLevels_.ForceSetDirty();
}

megamol::compositing_gl::OpticalFlow::~OpticalFlow() {
    this->Release();
}

bool megamol::compositing_gl::OpticalFlow::create() {

    // Create shader options
    auto const shdr_options = core::utility::make_path_shader_options(
        frontend_resources.get<megamol::frontend_resources::RuntimeConfig>());

    try {
        // Initialize compute shaders
        updateVShader_ = core::utility::make_glowl_shader(
            "update_v", shdr_options, std::filesystem::path("compositing_gl/OpicalFlow/update_v.comp.glsl"));

        updateUShader_ = core::utility::make_glowl_shader(
            "update_u", shdr_options, std::filesystem::path("compositing_gl/OpticalFlow/update_u.comp.glsl"));

        updatePShader_ = core::utility::make_glowl_shader(
            "update_p", shdr_options, std::filesystem::path("compositing_gl/OpticalFlow/update_p.comp.glsl"));

        computeChangeShader_ = core::utility::make_glowl_shader(
            "compute_change", shdr_options, std::filesystem::path("compositing_gl/OpticalFlow/compute_change.comp.glsl"));

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
    glowl::TextureLayout tex_layout{GL_RGBA32F, 1, 1, 1, GL_RGBA, GL_FLOAT, 1}; // Use appropriate format and type for your data

    // Initialize input textures
    I0_ = std::make_shared<glowl::Texture2D>("I0", tex_layout, nullptr);
    I1_ = std::make_shared<glowl::Texture2D>("I1", tex_layout, nullptr);

    // Initialize textures for intermediate variables
    uTexture_ = std::make_shared<glowl::Texture2D>("u_texture", tex_layout, nullptr);
    vTexture_ = std::make_shared<glowl::Texture2D>("v_texture", tex_layout, nullptr);
    pTexture_ = std::make_shared<glowl::Texture2D>("p_texture", tex_layout, nullptr);
    deltaUBuffer_ = std::make_shared<glowl::Texture2D>("delta_u_buffer", tex_layout, nullptr);

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
                          theta_.IsDirty() ||
                          tau_.IsDirty() ||
                          tolerance_.IsDirty() ||
                          maxIter_.IsDirty() ||
                          numLevels_.IsDirty();

    if (incomingChange) {
        ++version_;


        // Reset dirty flags
        lambda_.ResetDirty();
        theta_.ResetDirty();
        tau_.ResetDirty();
        tolerance_.ResetDirty();
        maxIter_.ResetDirty();
        numLevels_.ResetDirty();
    
        auto input_tex_2D = call_texture->getData();

        auto lambdaVal = lambda_.Param<core::param::FloatParam>()->Value();
        auto thetaVal = theta_.Param<core::param::FloatParam>()->Value();
        auto tauVal = tau_.Param<core::param::FloatParam>()->Value();
        auto toleranceVal = tolerance_.Param<core::param::FloatParam>()->Value();
        auto maxIterVal = maxIter_.Param<core::param::IntParam>()->Value();
        auto numLevels = numLevels_.Param<core::param::IntParam>()->Value();

        if(isFirstCall_) {
            isFirstCall_ = false;
            I0_->reload(input_tex_2D->getTextureLayout(), &input_tex_2D, true, numLevels);
            lhs_tc->setData(input_tex_2D, version_);
            return true;
        } 

        I1_->reload(input_tex_2D->getTextureLayout(), &input_tex_2D, true, numLevels);

        // Retrieve parameter values
        for(int level = numLevels -1; level >= 0; level--) {
            bool converged = false;
            int iteration = 0;

            while (!converged && iteration < maxIterVal) {
                
            }

            I0_->reload(I1_->getTextureLayout(), &I1_, true, level);
        }
    }

    lhs_tc->setData(outputTex_, version_);

    return true;
}

bool megamol::compositing_gl::OpticalFlow::getMetaDataCallback(core::Call& caller) {
    return true;
}

void megamol::compositing_gl::OpticalFlow::fitTextures(std::shared_ptr<glowl::Texture2D> source) {
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